/***********************************************************************************************************************
**
** Copyright (C) 2026 BaseALT Ltd. <org@basealt.ru>
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
***********************************************************************************************************************/

#include "targetingdialog.h"

#include "targetingfilterdefaults.h"
#include "ui_targetingdialog.h"

#include "common/basepreferencewidget.h"
#include "targetingfilteritem.h"
#include "targetingicons.h"
#include "targetingmodel.h"
#include "targetingselection.h"
#include "targetingwidgetfactory.h"

#include <mvvm/model/sessionitem.h>
#include <mvvm/model/sessionmodel.h>
#include <mvvm/model/tagrow.h>
#include <mvvm/viewmodel/viewmodel.h>

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QCoreApplication>
#include <QDataStream>
#include <QDialogButtonBox>
#include <QDropEvent>
#include <QHBoxLayout>
#include <QHash>
#include <QHeaderView>
#include <QIdentityProxyModel>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QShortcut>
#include <QSet>
#include <QStackedWidget>
#include <QStyledItemDelegate>
#include <QToolButton>
#include <QUuid>
#include <QVBoxLayout>

#include <algorithm>
#include <functional>

namespace preferences
{

namespace
{

void writeRecord(QDataStream &out, const TargetingFilterRecord &record)
{
    out << record.name << record.id << record.combinator
        << record.negated << record.disabled << record.attributes
        << static_cast<quint32>(record.children.size());
    for (const auto &child : record.children)
    {
        writeRecord(out, child);
    }
}

TargetingFilterRecord readRecord(QDataStream &in)
{
    TargetingFilterRecord record;
    in >> record.name >> record.id >> record.combinator
       >> record.negated >> record.disabled >> record.attributes;
    record.combinator = combinatorFromString(record.combinator);
    quint32 count = 0;
    in >> count;
    record.children.reserve(static_cast<int>(count));
    for (quint32 i = 0; i < count; ++i)
    {
        record.children.append(readRecord(in));
    }
    return record;
}

//! Recursively replace `id` attributes with fresh UUIDs (used after Paste).
void regenerateIds(TargetingFilterRecord &record)
{
    record.id = QUuid::createUuid().toString();
    for (auto &child : record.children)
    {
        regenerateIds(child);
    }
}

QByteArray encodeRecords(const QList<TargetingFilterRecord> &records)
{
    QByteArray buffer;
    QDataStream out(&buffer, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_12);
    out << static_cast<quint32>(records.size());
    for (const auto &record : records)
    {
        writeRecord(out, record);
    }
    return buffer;
}

QList<TargetingFilterRecord> decodeRecords(const QByteArray &buffer)
{
    QList<TargetingFilterRecord> out;
    QDataStream in(buffer);
    in.setVersion(QDataStream::Qt_5_12);
    quint32 count = 0;
    in >> count;
    out.reserve(static_cast<int>(count));
    for (quint32 i = 0; i < count; ++i)
    {
        out.append(readRecord(in));
    }
    return out;
}

//! Tag to use when inserting under the SessionModel root vs. a collection.
const std::string &childTagFor(const ModelView::SessionItem *parent,
                               const ModelView::SessionItem *root)
{
    return parent == root ? TargetingModel::kFiltersTag
                          : TargetingFilterItem::kChildrenTag;
}

//! Identity-proxy that drops every column past the first. The MVVM
//! `TopItemsViewModel` always allocates Name + Value columns; we want a
//! single-column list look in the targeting dialog, and hiding the
//! second column on the QTreeView still left it in the column-count and
//! occasionally crashed `setSectionResizeMode`. Restricting the column
//! count at the proxy level avoids both.
class SingleColumnProxy : public QIdentityProxyModel
{
public:
    explicit SingleColumnProxy(QObject *parent = nullptr)
        : QIdentityProxyModel(parent)
        , m_sourceToken(QUuid::createUuid().toString())
    {}

    void setMoveContext(ModelView::ViewModel *vm,
                        ModelView::SessionModel *model,
                        ModelView::SessionItem *root,
                        std::function<void(const QList<TargetingFilterItem *> &)> afterMove)
    {
        m_vm        = vm;
        m_model     = model;
        m_root      = root;
        m_afterMove = std::move(afterMove);
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const override
    {
        Q_UNUSED(parent);
        return 1;
    }

    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        auto f = QIdentityProxyModel::flags(index);
        if (index.isValid())
        {
            f |= Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
        }
        else
        {
            f |= Qt::ItemIsDropEnabled;
        }
        return f;
    }

    Qt::DropActions supportedDropActions() const override
    {
        return Qt::MoveAction;
    }

    Qt::DropActions supportedDragActions() const override
    {
        return Qt::MoveAction;
    }

    QStringList mimeTypes() const override
    {
        return {QString::fromLatin1(kTargetingInternalDragMimeType)};
    }

    QMimeData *mimeData(const QModelIndexList &indexes) const override
    {
        auto *mime = new QMimeData();
        QList<TargetingFilterItem *> items;
        QSet<TargetingFilterItem *> seen;
        for (const auto &idx : indexes)
        {
            if (!idx.isValid() || idx.column() != 0)
            {
                continue;
            }
            if (auto *item = itemFromProxyIndex(idx))
            {
                if (!seen.contains(item))
                {
                    seen.insert(item);
                    items.append(item);
                }
            }
        }

        mime->setData(QString::fromLatin1(kTargetingInternalDragMimeType),
                      encodeTargetingInternalDragData(m_sourceToken, items));
        return mime;
    }

    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row,
                         int column, const QModelIndex &parent) const override
    {
        Q_UNUSED(column);
        if (action == Qt::IgnoreAction)
        {
            return true;
        }

        QList<TargetingFilterItem *> sources;
        ModelView::SessionItem *targetParent = nullptr;
        int insertAt = row;
        return resolveDrop(data, action, row, parent, sources, targetParent, insertAt);
    }

    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row,
                      int column, const QModelIndex &parent) override
    {
        Q_UNUSED(column);
        if (action == Qt::IgnoreAction)
        {
            return true;
        }
        QList<TargetingFilterItem *> sources;
        ModelView::SessionItem *targetParent = nullptr;
        int insertAt = row;
        if (!resolveDrop(data, action, row, parent, sources, targetParent, insertAt))
        {
            return false;
        }

        struct MoveRecord
        {
            TargetingFilterItem *item;
            ModelView::SessionItem *parent;
            int row;
            TargetingFilterRecord record;
        };

        QList<MoveRecord> moves;
        moves.reserve(sources.size());
        for (auto *source : sources)
        {
            moves.append({source, source->parent(), source->tagRow().row, source->toRecord()});
        }

        insertAt = adjustedTargetingDropRow(insertAt, targetParent, sources);

        auto removalOrder = moves;
        std::sort(removalOrder.begin(), removalOrder.end(),
                  [](const MoveRecord &a, const MoveRecord &b) {
                      if (a.parent == b.parent)
                      {
                          return a.row > b.row;
                      }
                      return targetingItemDepth(a.item) > targetingItemDepth(b.item);
                  });
        for (const auto &move : removalOrder)
        {
            m_model->removeItem(move.parent, move.item->tagRow());
        }

        QList<TargetingFilterItem *> insertedItems;
        insertedItems.reserve(moves.size());
        const auto &targetTag = childTagFor(targetParent, m_root);
        for (const auto &move : moves)
        {
            auto *inserted = m_model->insertItem<TargetingFilterItem>(
                targetParent, {targetTag, insertAt++});
            inserted->loadRecord(move.record);
            insertedItems.append(inserted);
        }

        if (m_afterMove)
        {
            m_afterMove(insertedItems);
        }
        return true;
    }

private:
    bool resolveDrop(const QMimeData *data, Qt::DropAction action, int row,
                     const QModelIndex &parent,
                     QList<TargetingFilterItem *> &sources,
                     ModelView::SessionItem *&targetParent,
                     int &insertAt) const
    {
        if (action != Qt::MoveAction || !data
            || !data->hasFormat(QString::fromLatin1(kTargetingInternalDragMimeType))
            || !m_vm || !m_model || !m_root)
        {
            return false;
        }

        if (!resolveTargetingInternalDragData(data, m_sourceToken, m_root, sources))
        {
            return false;
        }

        insertAt = row;
        if (row < 0 && parent.isValid())
        {
            auto *dropTarget = itemFromProxyIndex(parent);
            if (!dropTarget || !dropTarget->isCollection())
            {
                return false;
            }
            targetParent = dropTarget;
            insertAt = targetParent->itemCount(TargetingFilterItem::kChildrenTag);
        }
        else
        {
            targetParent = parent.isValid()
                ? m_vm->sessionItemFromIndex(mapToSource(parent))
                : m_root;
            if (!targetParent)
            {
                return false;
            }
            const auto &tag = childTagFor(targetParent, m_root);
            if (insertAt < 0)
            {
                insertAt = targetParent->itemCount(tag);
            }
        }

        for (auto *source : sources)
        {
            if (isTargetingAncestorOf(source, targetParent))
            {
                return false;
            }
        }
        return true;
    }

    TargetingFilterItem *itemFromProxyIndex(const QModelIndex &index) const
    {
        if (!m_vm || !index.isValid())
        {
            return nullptr;
        }
        const QModelIndex srcIdx = mapToSource(index);
        return dynamic_cast<TargetingFilterItem *>(m_vm->sessionItemFromIndex(srcIdx));
    }

    ModelView::ViewModel *m_vm{nullptr};
    ModelView::SessionModel *m_model{nullptr};
    ModelView::SessionItem *m_root{nullptr};
    QString m_sourceToken;
    std::function<void(const QList<TargetingFilterItem *> &)> m_afterMove;
};

//! Item delegate that paints the per-filter icon for column 0 of the
//! targeting tree. The MVVM ViewModel doesn't surface `Qt::DecorationRole`
//! by default, so we resolve the icon from the underlying SessionItem
//! ourselves on every paint.
class TargetingIconDelegate : public QStyledItemDelegate
{
public:
    explicit TargetingIconDelegate(ModelView::ViewModel *vm,
                                   QAbstractProxyModel *proxy,
                                   QObject *parent = nullptr)
        : QStyledItemDelegate(parent), m_vm(vm), m_proxy(proxy)
    {}

protected:
    void initStyleOption(QStyleOptionViewItem *option,
                         const QModelIndex &index) const override
    {
        QStyledItemDelegate::initStyleOption(option, index);
        if (!m_vm || index.column() != 0)
        {
            return;
        }
        const QModelIndex srcIdx = m_proxy ? m_proxy->mapToSource(index) : index;
        if (auto *item = dynamic_cast<TargetingFilterItem *>(m_vm->sessionItemFromIndex(srcIdx)))
        {
            option->features |= QStyleOptionViewItem::HasDecoration;
            option->icon      = TargetingIcons::iconFor(item->filterName());
        }
    }

private:
    ModelView::ViewModel *m_vm;
    QAbstractProxyModel *m_proxy;
};

} // namespace

TargetingDialog::TargetingDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TargetingDialog())
    , m_model(std::make_unique<TargetingModel>())
    , m_widgetFactory(std::make_unique<TargetingWidgetFactory>())
{
    ui->setupUi(this);

    // Single-column proxy: drops the MVVM-default Value column entirely
    // so the treeView never has more than one column to render.
    m_proxy = new SingleColumnProxy(this);
    static_cast<SingleColumnProxy *>(m_proxy)->setMoveContext(
        m_model->viewModel(), m_model->sessionModel(), m_model->rootFilterParent(),
        [this](const QList<TargetingFilterItem *> &items) {
            selectItems(items);
            finishMutation();
        });
    m_proxy->setSourceModel(m_model->viewModel());

    ui->treeView->setModel(m_proxy);
    ui->treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->treeView->setDragEnabled(true);
    ui->treeView->setAcceptDrops(true);
    ui->treeView->viewport()->setAcceptDrops(true);
    ui->treeView->setDragDropMode(QAbstractItemView::InternalMove);
    ui->treeView->setDragDropOverwriteMode(false);
    ui->treeView->setDefaultDropAction(Qt::MoveAction);
    ui->treeView->setDropIndicatorShown(true);
    ui->treeView->setStyleSheet(QStringLiteral(
        "QTreeView::drop-indicator {"
        "  height: 2px;"
        "  background: palette(highlight);"
        "}"));
    ui->treeView->setUniformRowHeights(true);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeView->setItemDelegate(
        new TargetingIconDelegate(m_model->viewModel(), m_proxy, this));
    if (auto *h = ui->treeView->header())
    {
        h->setVisible(false);
    }
    connect(ui->treeView, &QTreeView::customContextMenuRequested,
            this, &TargetingDialog::onTreeContextMenu);
    if (auto *sel = ui->treeView->selectionModel())
    {
        connect(sel, &QItemSelectionModel::selectionChanged, this,
                [this](const QItemSelection &, const QItemSelection &) { onSelectionChanged(); });
    }

    buildToolbar();

    // Standard list shortcuts. Scoped to the dialog window so they fire
    // whenever any descendant has focus (the inline editor pane never
    // overrides them in normal use).
    auto bindShortcut = [this](QKeySequence::StandardKey seq, void (TargetingDialog::*slot)()) {
        auto *sc = new QShortcut(QKeySequence(seq), this);
        sc->setContext(Qt::WidgetWithChildrenShortcut);
        connect(sc, &QShortcut::activated, this, slot);
    };
    bindShortcut(QKeySequence::Delete, &TargetingDialog::deleteSelected);
    bindShortcut(QKeySequence::Cut,    &TargetingDialog::cutSelected);
    bindShortcut(QKeySequence::Copy,   &TargetingDialog::copySelected);
    bindShortcut(QKeySequence::Paste,  &TargetingDialog::pasteAtSelection);

    updateActionStates();
}

TargetingDialog::~TargetingDialog()
{
    delete ui;
}

void TargetingDialog::setContainer(const TargetingContainer &container)
{
    m_model->fromContainer(container);

    // Force the proxy (and the treeView wired through it) to fully
    // re-read the source view model. Without this kick the tree stays
    // empty until the user adds or removes a row — TopItemsViewModel
    // sometimes skips items inserted before its first paint.
    if (m_proxy)
    {
        m_proxy->setSourceModel(nullptr);
        m_proxy->setSourceModel(m_model->viewModel());
        // setSourceModel destroys/recreates the selection model — re-bind.
        if (auto *sel = ui->treeView->selectionModel())
        {
            connect(sel, &QItemSelectionModel::selectionChanged, this,
                    [this](const QItemSelection &, const QItemSelection &) {
                        onSelectionChanged();
                    });
        }
    }

    ui->treeView->expandAll();

    // Always have a row selected so the inline editor pane is never empty.
    if (m_proxy && m_proxy->rowCount() > 0)
    {
        const QModelIndex first = m_proxy->index(0, 0);
        if (first.isValid())
        {
            ui->treeView->setCurrentIndex(first);
        }
    }
    updateActionStates();
}

TargetingContainer TargetingDialog::container() const
{
    return m_model->toContainer();
}

QStringList TargetingDialog::filterCatalog()
{
    // XSD element names from src/plugins/preferences/schemas/targetingschema.xsd,
    // minus FilterRunOnce (reserved for the apply-once checkbox) and
    // FilterCollection (added via the Wrap action, not the New Item menu).
    return {
        QStringLiteral("FilterBattery"),
        QStringLiteral("FilterComputer"),
        QStringLiteral("FilterCpu"),
        QStringLiteral("FilterDate"),
        QStringLiteral("FilterDisk"),
        QStringLiteral("FilterDomain"),
        QStringLiteral("FilterDun"),
        QStringLiteral("FilterVariable"),
        QStringLiteral("FilterFile"),
        QStringLiteral("FilterGroup"),
        QStringLiteral("FilterIpRange"),
        QStringLiteral("FilterLanguage"),
        QStringLiteral("FilterLdap"),
        QStringLiteral("FilterMacRange"),
        QStringLiteral("FilterMsi"),
        QStringLiteral("FilterOrgUnit"),
        QStringLiteral("FilterOs"),
        QStringLiteral("FilterPcmcia"),
        QStringLiteral("FilterPortable"),
        QStringLiteral("FilterProcMode"),
        QStringLiteral("FilterRam"),
        QStringLiteral("FilterRegistry"),
        QStringLiteral("FilterSite"),
        QStringLiteral("FilterTerminal"),
        QStringLiteral("FilterTime"),
        QStringLiteral("FilterUser"),
        QStringLiteral("FilterWmi"),
    };
}

QString TargetingDialog::filterDisplayName(const QString &filterName)
{
    static const QHash<QString, const char *> names = {
        {QStringLiteral("FilterBattery"),  QT_TRANSLATE_NOOP("preferences::TargetingDialog", "Battery Present")},
        {QStringLiteral("FilterComputer"), QT_TRANSLATE_NOOP("preferences::TargetingDialog", "Computer Name")},
        {QStringLiteral("FilterCpu"),      QT_TRANSLATE_NOOP("preferences::TargetingDialog", "CPU Speed")},
        {QStringLiteral("FilterDate"),     QT_TRANSLATE_NOOP("preferences::TargetingDialog", "Date Match")},
        {QStringLiteral("FilterDisk"),     QT_TRANSLATE_NOOP("preferences::TargetingDialog", "Disk Space")},
        {QStringLiteral("FilterDomain"),   QT_TRANSLATE_NOOP("preferences::TargetingDialog", "Domain")},
        {QStringLiteral("FilterDun"),      QT_TRANSLATE_NOOP("preferences::TargetingDialog", "Dial-Up Networking")},
        {QStringLiteral("FilterVariable"), QT_TRANSLATE_NOOP("preferences::TargetingDialog", "Environment Variable")},
        {QStringLiteral("FilterFile"),     QT_TRANSLATE_NOOP("preferences::TargetingDialog", "File Match")},
        {QStringLiteral("FilterGroup"),    QT_TRANSLATE_NOOP("preferences::TargetingDialog", "Security Group")},
        {QStringLiteral("FilterIpRange"),  QT_TRANSLATE_NOOP("preferences::TargetingDialog", "IP Address Range")},
        {QStringLiteral("FilterLanguage"), QT_TRANSLATE_NOOP("preferences::TargetingDialog", "Language")},
        {QStringLiteral("FilterLdap"),     QT_TRANSLATE_NOOP("preferences::TargetingDialog", "LDAP Query")},
        {QStringLiteral("FilterMacRange"), QT_TRANSLATE_NOOP("preferences::TargetingDialog", "MAC Address Range")},
        {QStringLiteral("FilterMsi"),      QT_TRANSLATE_NOOP("preferences::TargetingDialog", "MSI Query")},
        {QStringLiteral("FilterOrgUnit"),  QT_TRANSLATE_NOOP("preferences::TargetingDialog", "Organizational Unit")},
        {QStringLiteral("FilterOs"),       QT_TRANSLATE_NOOP("preferences::TargetingDialog", "Operating System")},
        {QStringLiteral("FilterPcmcia"),   QT_TRANSLATE_NOOP("preferences::TargetingDialog", "PCMCIA Present")},
        {QStringLiteral("FilterPortable"), QT_TRANSLATE_NOOP("preferences::TargetingDialog", "Portable Computer")},
        {QStringLiteral("FilterProcMode"), QT_TRANSLATE_NOOP("preferences::TargetingDialog", "Processing Mode")},
        {QStringLiteral("FilterRam"),      QT_TRANSLATE_NOOP("preferences::TargetingDialog", "RAM")},
        {QStringLiteral("FilterRegistry"), QT_TRANSLATE_NOOP("preferences::TargetingDialog", "Registry Match")},
        {QStringLiteral("FilterSite"),     QT_TRANSLATE_NOOP("preferences::TargetingDialog", "Site")},
        {QStringLiteral("FilterTerminal"), QT_TRANSLATE_NOOP("preferences::TargetingDialog", "Terminal Session")},
        {QStringLiteral("FilterTime"),     QT_TRANSLATE_NOOP("preferences::TargetingDialog", "Time Range")},
        {QStringLiteral("FilterUser"),     QT_TRANSLATE_NOOP("preferences::TargetingDialog", "User")},
        {QStringLiteral("FilterWmi"),      QT_TRANSLATE_NOOP("preferences::TargetingDialog", "WMI Query")},
    };
    const auto it = names.constFind(filterName);
    if (it != names.constEnd())
    {
        return QCoreApplication::translate("preferences::TargetingDialog", it.value());
    }
    return filterName.startsWith(QLatin1String("Filter")) ? filterName.mid(6) : filterName;
}

void TargetingDialog::buildToolbar()
{
    ui->toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);

    // 1) Create Item — text + arrow split-button with the filter catalog menu.
    auto *createBtn = new QToolButton(this);
    createBtn->setIcon(TargetingIcons::toolbarIcon(QStringLiteral("list-add")));
    createBtn->setText(tr("Create Item"));
    createBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    createBtn->setPopupMode(QToolButton::MenuButtonPopup);
    auto *createMenu = new QMenu(createBtn);
    for (const auto &name : filterCatalog())
    {
        const QString label = filterDisplayName(name);
        auto *action = createMenu->addAction(TargetingIcons::iconFor(name), label);
        connect(action, &QAction::triggered, this, [this, name]() { addFilter(name); });
    }
    createBtn->setMenu(createMenu);
    // Clicking the text-area also opens the menu, for discoverability.
    connect(createBtn, &QToolButton::clicked, createBtn, &QToolButton::showMenu);
    ui->toolBar->addWidget(createBtn);

    // 2) Add Collection — first-class icon-with-text action.
    m_actAddCollection = ui->toolBar->addAction(
        TargetingIcons::toolbarIcon(QStringLiteral("folder-new")), tr("Add Collection"));
    m_actAddCollection->setToolTip(tr("Add Collection"));
    connect(m_actAddCollection, &QAction::triggered, this, &TargetingDialog::addCollection);
    if (auto *btn = qobject_cast<QToolButton *>(ui->toolBar->widgetForAction(m_actAddCollection)))
    {
        btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    }

    ui->toolBar->addSeparator();

    // 3) Item Options compound: combinator + negate dropdowns with a label.
    auto *itemOptions = new QWidget(this);
    auto *optsLayout  = new QHBoxLayout(itemOptions);
    optsLayout->setContentsMargins(4, 0, 4, 0);
    optsLayout->setSpacing(4);
    optsLayout->addWidget(new QLabel(tr("Options:"), itemOptions));

    m_combCombinator = new QComboBox(itemOptions);
    m_combCombinator->addItem(tr("AND"));
    m_combCombinator->addItem(tr("OR"));
    optsLayout->addWidget(m_combCombinator);
    connect(m_combCombinator, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TargetingDialog::onCombinatorComboChanged);

    m_combNegated = new QComboBox(itemOptions);
    m_combNegated->addItem(tr("is"));
    m_combNegated->addItem(tr("is not"));
    optsLayout->addWidget(m_combNegated);
    connect(m_combNegated, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TargetingDialog::onNegatedComboChanged);

    ui->toolBar->addWidget(itemOptions);

    ui->toolBar->addSeparator();

    auto addToolbarAction = [this](const QString &iconName, const QString &text) {
        auto *action = ui->toolBar->addAction(TargetingIcons::toolbarIcon(iconName), text);
        action->setToolTip(text);
        return action;
    };

    // 4) Up / Down — icon-only.
    m_actUp   = addToolbarAction(QStringLiteral("go-up"),   tr("Up"));
    m_actDown = addToolbarAction(QStringLiteral("go-down"), tr("Down"));
    connect(m_actUp,   &QAction::triggered, this, &TargetingDialog::moveUp);
    connect(m_actDown, &QAction::triggered, this, &TargetingDialog::moveDown);

    ui->toolBar->addSeparator();

    // 5) Cut / Copy / Paste — icon-only with localised tooltips.
    m_actCut   = addToolbarAction(QStringLiteral("edit-cut"),   tr("Cut"));
    m_actCopy  = addToolbarAction(QStringLiteral("edit-copy"),  tr("Copy"));
    m_actPaste = addToolbarAction(QStringLiteral("edit-paste"), tr("Paste"));
    connect(m_actCut,   &QAction::triggered, this, &TargetingDialog::cutSelected);
    connect(m_actCopy,  &QAction::triggered, this, &TargetingDialog::copySelected);
    connect(m_actPaste, &QAction::triggered, this, &TargetingDialog::pasteAtSelection);

    ui->toolBar->addSeparator();

    // 6) Delete — icon AND text label.
    m_actDelete = addToolbarAction(QStringLiteral("edit-delete"), tr("Delete"));
    connect(m_actDelete, &QAction::triggered, this, &TargetingDialog::deleteSelected);
    if (auto *btn = qobject_cast<QToolButton *>(ui->toolBar->widgetForAction(m_actDelete)))
    {
        btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    }

    // Off-toolbar actions (kept as members for the row context menu).
    m_actWrap = new QAction(TargetingIcons::toolbarIcon(QStringLiteral("folder-new")),
                            tr("Wrap in Collection"), this);
    connect(m_actWrap, &QAction::triggered, this, &TargetingDialog::wrapInCollection);

    m_actUnwrap = new QAction(TargetingIcons::toolbarIcon(QStringLiteral("edit-clear")),
                              tr("Unwrap Collection"), this);
    connect(m_actUnwrap, &QAction::triggered, this, &TargetingDialog::unwrapCollection);

    m_actNegate = new QAction(TargetingIcons::toolbarIcon(QStringLiteral("edit-undo")),
                              tr("Negate"), this);
    m_actNegate->setCheckable(true);
    connect(m_actNegate, &QAction::triggered, this, &TargetingDialog::toggleNegate);

    m_actCombAnd = new QAction(tr("AND"), this);
    m_actCombAnd->setCheckable(true);
    connect(m_actCombAnd, &QAction::triggered, this, &TargetingDialog::setCombinatorAnd);

    m_actCombOr = new QAction(tr("OR"), this);
    m_actCombOr->setCheckable(true);
    connect(m_actCombOr, &QAction::triggered, this, &TargetingDialog::setCombinatorOr);
}

QList<TargetingFilterItem *> TargetingDialog::selectedItems() const
{
    QList<TargetingFilterItem *> out;
    auto *vm  = m_model->viewModel();
    auto *sel = ui->treeView->selectionModel();
    if (!vm || !sel)
    {
        return out;
    }
    QSet<TargetingFilterItem *> seen;
    for (const auto &proxyIdx : sel->selectedRows())
    {
        const QModelIndex srcIdx = m_proxy ? m_proxy->mapToSource(proxyIdx) : proxyIdx;
        if (auto *item = dynamic_cast<TargetingFilterItem *>(vm->sessionItemFromIndex(srcIdx)))
        {
            if (!seen.contains(item))
            {
                seen.insert(item);
                out.append(item);
            }
        }
    }
    return out;
}

QList<TargetingFilterItem *> TargetingDialog::selectedSiblings() const
{
    const auto items = selectedItems();
    if (items.isEmpty())
    {
        return {};
    }
    auto *parent = items.first()->parent();
    QList<TargetingFilterItem *> siblings;
    for (auto *it : items)
    {
        if (it->parent() != parent)
        {
            return {};
        }
        siblings.append(it);
    }
    std::sort(siblings.begin(), siblings.end(),
              [](TargetingFilterItem *a, TargetingFilterItem *b) {
                  return a->tagRow().row < b->tagRow().row;
              });
    return siblings;
}

QList<TargetingFilterItem *> TargetingDialog::selectedContiguousSiblings() const
{
    auto siblings = selectedSiblings();
    if (siblings.size() < 2)
    {
        return {};
    }
    for (int i = 1; i < siblings.size(); ++i)
    {
        if (siblings.at(i)->tagRow().row != siblings.at(i - 1)->tagRow().row + 1)
        {
            return {};
        }
    }
    return siblings;
}

TargetingFilterItem *TargetingDialog::itemFromProxyIndex(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return nullptr;
    }
    const QModelIndex srcIdx = m_proxy ? m_proxy->mapToSource(index) : index;
    return dynamic_cast<TargetingFilterItem *>(
        m_model->viewModel()->sessionItemFromIndex(srcIdx));
}

QModelIndex TargetingDialog::proxyIndexForItem(TargetingFilterItem *item) const
{
    if (!item)
    {
        return {};
    }
    const auto indices = m_model->viewModel()->indexOfSessionItem(item);
    if (indices.empty())
    {
        return {};
    }
    return m_proxy ? m_proxy->mapFromSource(indices.front()) : indices.front();
}

void TargetingDialog::selectItem(TargetingFilterItem *item)
{
    const QModelIndex idx = proxyIndexForItem(item);
    if (idx.isValid())
    {
        ui->treeView->setCurrentIndex(idx);
    }
}

void TargetingDialog::selectItems(const QList<TargetingFilterItem *> &items)
{
    auto *selectionModel = ui->treeView->selectionModel();
    if (!selectionModel || items.isEmpty())
    {
        return;
    }

    selectionModel->clearSelection();
    QModelIndex current;
    for (auto *item : items)
    {
        const QModelIndex idx = proxyIndexForItem(item);
        if (!idx.isValid())
        {
            continue;
        }
        selectionModel->select(idx, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        current = idx;
    }

    if (current.isValid())
    {
        selectionModel->setCurrentIndex(current, QItemSelectionModel::NoUpdate);
        ui->treeView->scrollTo(current);
    }
}

void TargetingDialog::refreshLabels(TargetingFilterItem *root)
{
    std::function<void(ModelView::SessionItem *, const std::string &)> refreshChildren =
        [&refreshChildren](ModelView::SessionItem *parent, const std::string &tag) {
        for (auto *child : parent->getItems(tag))
        {
            if (auto *filter = dynamic_cast<TargetingFilterItem *>(child))
            {
                filter->refreshDisplayName();
                refreshChildren(filter, TargetingFilterItem::kChildrenTag);
            }
        }
    };

    if (root)
    {
        root->refreshDisplayName();
        refreshChildren(root, TargetingFilterItem::kChildrenTag);
        return;
    }

    refreshChildren(m_model->rootFilterParent(), TargetingModel::kFiltersTag);
}

void TargetingDialog::finishMutation(TargetingFilterItem *refreshRoot)
{
    refreshLabels(refreshRoot);
    updateActionStates();
}

void TargetingDialog::computeInsertionSlot(ModelView::SessionItem *&parent,
                                            int &insertRow,
                                            std::string &tag) const
{
    auto items    = selectedItems();
    auto *current = items.isEmpty() ? nullptr : items.last();
    parent        = nullptr;
    insertRow     = -1;

    if (current && current->isCollection())
    {
        parent    = current;
        insertRow = current->itemCount(TargetingFilterItem::kChildrenTag);
    }
    else if (current)
    {
        parent    = current->parent();
        insertRow = current->tagRow().row + 1;
    }

    if (!parent)
    {
        parent    = m_model->rootFilterParent();
        insertRow = parent->itemCount(TargetingModel::kFiltersTag);
    }
    tag = childTagFor(parent, m_model->rootFilterParent());
}

void TargetingDialog::addFilter(const QString &filterName)
{
    auto *vm = m_model->viewModel();
    ModelView::SessionItem *parent = nullptr;
    int insertRow = -1;
    std::string tag;
    computeInsertionSlot(parent, insertRow, tag);

    auto *item = m_model->sessionModel()->insertItem<TargetingFilterItem>(parent, {tag, insertRow});

    TargetingFilterRecord record;
    record.name       = filterName;
    record.id         = QUuid::createUuid().toString();
    record.attributes = defaultTargetingFilterExtras(filterName);
    item->loadRecord(record);

    const auto indices = vm->indexOfSessionItem(item);
    if (!indices.empty())
    {
        ui->treeView->setCurrentIndex(m_proxy ? m_proxy->mapFromSource(indices.front()) : indices.front());
    }
    // Selection-change handler will populate the inline editor pane.
    finishMutation(dynamic_cast<TargetingFilterItem *>(parent));
}

void TargetingDialog::addCollection()
{
    auto *vm = m_model->viewModel();
    ModelView::SessionItem *parent = nullptr;
    int insertRow = -1;
    std::string tag;
    computeInsertionSlot(parent, insertRow, tag);

    auto *item = m_model->sessionModel()->insertItem<TargetingFilterItem>(parent, {tag, insertRow});
    TargetingFilterRecord record;
    record.name       = QStringLiteral("FilterCollection");
    record.id         = QUuid::createUuid().toString();
    record.combinator = QStringLiteral("AND");
    item->loadRecord(record);

    const auto indices = vm->indexOfSessionItem(item);
    if (!indices.empty())
    {
        ui->treeView->setCurrentIndex(m_proxy ? m_proxy->mapFromSource(indices.front()) : indices.front());
    }
    if (!indices.empty())
    {
        ui->treeView->expand(m_proxy ? m_proxy->mapFromSource(indices.front()) : indices.front());
    }
    finishMutation(dynamic_cast<TargetingFilterItem *>(parent));
}

void TargetingDialog::deleteSelected()
{
    removeItems(selectedItems());
}

void TargetingDialog::copyItemsToClipboard(const QList<TargetingFilterItem *> &items) const
{
    const auto normalized = normalizeTargetingSelection(items);
    if (normalized.isEmpty())
    {
        return;
    }

    QList<TargetingFilterRecord> records;
    records.reserve(normalized.size());
    for (auto *item : normalized)
    {
        records.append(item->toRecord());
    }

    auto *mime = new QMimeData();
    mime->setData(QString::fromLatin1(kTargetingClipboardMimeType), encodeRecords(records));
    QApplication::clipboard()->setMimeData(mime);
}

void TargetingDialog::removeItems(const QList<TargetingFilterItem *> &items)
{
    const auto targets = targetingRemovalOrder(items);
    if (targets.isEmpty())
    {
        return;
    }

    auto *model = m_model->sessionModel();
    for (auto *item : targets)
    {
        model->removeItem(item->parent(), item->tagRow());
    }

    // Always keep something selected after a delete so the inline editor
    // pane never goes blank — pick the first top-level filter, if any.
    if (m_proxy && m_proxy->rowCount() > 0)
    {
        const QModelIndex first = m_proxy->index(0, 0);
        if (first.isValid())
        {
            ui->treeView->setCurrentIndex(first);
        }
    }
    finishMutation();
}

void TargetingDialog::moveUp()
{
    auto *vm    = m_model->viewModel();
    auto items  = selectedItems();
    auto *item  = items.isEmpty() ? nullptr : items.last();
    if (!item)
    {
        return;
    }
    const auto tr = item->tagRow();
    if (tr.row <= 0)
    {
        return;
    }
    m_model->sessionModel()->moveItem(item, item->parent(), {tr.tag, tr.row - 1});

    const auto indices = vm->indexOfSessionItem(item);
    if (!indices.empty())
    {
        ui->treeView->setCurrentIndex(m_proxy ? m_proxy->mapFromSource(indices.front()) : indices.front());
    }
    finishMutation(dynamic_cast<TargetingFilterItem *>(item->parent()));
}

void TargetingDialog::moveDown()
{
    auto *vm   = m_model->viewModel();
    auto items = selectedItems();
    auto *item = items.isEmpty() ? nullptr : items.last();
    if (!item)
    {
        return;
    }
    auto *parent  = item->parent();
    const auto tr = item->tagRow();
    if (tr.row + 1 >= parent->itemCount(tr.tag))
    {
        return;
    }
    m_model->sessionModel()->moveItem(item, parent, {tr.tag, tr.row + 1});

    const auto indices = vm->indexOfSessionItem(item);
    if (!indices.empty())
    {
        ui->treeView->setCurrentIndex(m_proxy ? m_proxy->mapFromSource(indices.front()) : indices.front());
    }
    finishMutation(dynamic_cast<TargetingFilterItem *>(parent));
}

void TargetingDialog::cutSelected()
{
    const auto items = normalizeTargetingSelection(selectedItems());
    copyItemsToClipboard(items);
    removeItems(items);
}

void TargetingDialog::copySelected()
{
    copyItemsToClipboard(selectedItems());
}

void TargetingDialog::pasteAtSelection()
{
    const auto *mime = QApplication::clipboard()->mimeData();
    if (!mime || !mime->hasFormat(QString::fromLatin1(kTargetingClipboardMimeType)))
    {
        return;
    }

    auto records = decodeRecords(mime->data(QString::fromLatin1(kTargetingClipboardMimeType)));
    if (records.isEmpty())
    {
        return;
    }

    auto items     = selectedItems();
    auto *current  = items.isEmpty() ? nullptr : items.last();
    auto *parent   = current ? current->parent() : m_model->rootFilterParent();
    int insertAt   = current ? current->tagRow().row + 1
                             : parent->itemCount(childTagFor(parent, m_model->rootFilterParent()));

    if (current && current->isCollection())
    {
        parent   = current;
        insertAt = parent->itemCount(TargetingFilterItem::kChildrenTag);
    }

    const std::string &tag = childTagFor(parent, m_model->rootFilterParent());
    auto *model = m_model->sessionModel();

    TargetingFilterItem *lastInserted = nullptr;
    for (auto &record : records)
    {
        regenerateIds(record);
        auto *node = model->insertItem<TargetingFilterItem>(parent, {tag, insertAt++});
        node->loadRecord(record);
        lastInserted = node;
    }

    if (lastInserted)
    {
        const auto indices = m_model->viewModel()->indexOfSessionItem(lastInserted);
        if (!indices.empty())
        {
            ui->treeView->setCurrentIndex(m_proxy ? m_proxy->mapFromSource(indices.front()) : indices.front());
        }
    }
    ui->treeView->expandAll();
    finishMutation(dynamic_cast<TargetingFilterItem *>(parent));
}

void TargetingDialog::wrapInCollection()
{
    const auto siblings = selectedContiguousSiblings();
    if (siblings.isEmpty())
    {
        return;
    }

    // Capture record snapshots and the insertion slot before mutating.
    QList<TargetingFilterRecord> records;
    records.reserve(siblings.size());
    for (auto *it : siblings)
    {
        records.append(it->toRecord());
    }

    auto *parent          = siblings.first()->parent();
    const int slotRow     = siblings.first()->tagRow().row;
    const std::string tag = childTagFor(parent, m_model->rootFilterParent());

    auto *model = m_model->sessionModel();

    // Remove originals (deepest first).
    auto removalOrder = siblings;
    std::sort(removalOrder.begin(), removalOrder.end(),
              [](TargetingFilterItem *a, TargetingFilterItem *b) {
                  return a->tagRow().row > b->tagRow().row;
              });
    for (auto *it : removalOrder)
    {
        model->removeItem(parent, it->tagRow());
    }

    // Insert the new collection in their place.
    auto *collection = model->insertItem<TargetingFilterItem>(parent, {tag, slotRow});
    TargetingFilterRecord collectionRecord;
    collectionRecord.name     = QStringLiteral("FilterCollection");
    collectionRecord.id       = QUuid::createUuid().toString();
    collectionRecord.children = records;
    collection->loadRecord(collectionRecord);

    const auto indices = m_model->viewModel()->indexOfSessionItem(collection);
    if (!indices.empty())
    {
        ui->treeView->setCurrentIndex(m_proxy ? m_proxy->mapFromSource(indices.front()) : indices.front());
    }
    ui->treeView->expandAll();
    finishMutation(dynamic_cast<TargetingFilterItem *>(parent));
}

void TargetingDialog::unwrapCollection()
{
    const auto items = selectedItems();
    if (items.size() != 1)
    {
        return;
    }
    auto *collection = items.first();
    if (!collection->isCollection())
    {
        return;
    }

    auto *parent          = collection->parent();
    const auto srcTag     = collection->tagRow();
    const std::string dst = childTagFor(parent, m_model->rootFilterParent());
    auto *model           = m_model->sessionModel();

    int insertAt = srcTag.row;
    TargetingFilterItem *firstMoved = nullptr;
    while (collection->itemCount(TargetingFilterItem::kChildrenTag) > 0)
    {
        auto *childItem = collection->getItem(TargetingFilterItem::kChildrenTag, 0);
        auto *child = dynamic_cast<TargetingFilterItem *>(childItem);
        model->moveItem(childItem, parent, {dst, insertAt});
        if (!firstMoved)
        {
            firstMoved = child;
        }
        ++insertAt;
    }
    model->removeItem(parent, collection->tagRow());

    selectItem(firstMoved);
    finishMutation(dynamic_cast<TargetingFilterItem *>(parent));
}

void TargetingDialog::toggleNegate()
{
    for (auto *item : selectedItems())
    {
        item->setNegated(!item->negated());
    }
    updateActionStates();
}

void TargetingDialog::setCombinatorAnd()
{
    for (auto *item : selectedItems())
    {
        if (item->tagRow().row == 0)
        {
            continue; // First row of each parent has no predecessor.
        }
        item->setCombinator(QStringLiteral("AND"));
    }
    updateActionStates();
}

void TargetingDialog::setCombinatorOr()
{
    for (auto *item : selectedItems())
    {
        if (item->tagRow().row == 0)
        {
            continue;
        }
        item->setCombinator(QStringLiteral("OR"));
    }
    updateActionStates();
}

void TargetingDialog::showInlineEditor(TargetingFilterItem *item)
{
    // Index 0 of editorStack is always the placeholder page.
    if (!item || item->isCollection())
    {
        ui->editorStack->setCurrentIndex(0);
        return;
    }

    const QString name = item->filterName();
    auto cached = m_editorCache.constFind(name);
    BasePreferenceWidget *widget = (cached != m_editorCache.constEnd()) ? cached.value() : nullptr;

    if (!widget)
    {
        auto editor = m_widgetFactory->create(name);
        if (!editor)
        {
            // No registered editor for this filter type — show placeholder.
            ui->editorStack->setCurrentIndex(0);
            return;
        }
        widget = editor.release();
        widget->setParent(ui->editorStack);
        ui->editorStack->addWidget(widget);
        m_editorCache.insert(name, widget);
    }

    widget->setItem(item);
    ui->editorStack->setCurrentWidget(widget);
}

void TargetingDialog::onCombinatorComboChanged(int index)
{
    const auto items = selectedItems();
    if (items.size() != 1)
    {
        return;
    }
    auto *item = items.first();
    if (item->tagRow().row == 0)
    {
        return; // First row has no predecessor.
    }
    item->setCombinator(index == 0 ? QStringLiteral("AND") : QStringLiteral("OR"));
}

void TargetingDialog::onNegatedComboChanged(int index)
{
    const auto items = selectedItems();
    if (items.size() != 1)
    {
        return;
    }
    items.first()->setNegated(index == 1);
}

void TargetingDialog::onTreeContextMenu(const QPoint &pos)
{
    QMenu menu(this);

    const QModelIndex hit = ui->treeView->indexAt(pos);

    auto addAct = [&menu](const QString &text, QAction *src) {
        if (!src)
        {
            return;
        }
        auto *act = menu.addAction(text);
        act->setEnabled(src->isEnabled());
        act->setCheckable(src->isCheckable());
        act->setChecked(src->isChecked());
        QObject::connect(act, &QAction::triggered, src, &QAction::trigger);
    };

    if (!hit.isValid())
    {
        // Empty-area menu: creation actions.
        auto *createMenu = menu.addMenu(tr("Create Item"));
        for (const auto &name : filterCatalog())
        {
            const QString label = filterDisplayName(name);
            auto *action = createMenu->addAction(TargetingIcons::iconFor(name), label);
            connect(action, &QAction::triggered, this, [this, name]() { addFilter(name); });
        }
        addAct(tr("Add Collection"), m_actAddCollection);
        menu.addSeparator();
        addAct(tr("Paste"), m_actPaste);
        menu.exec(ui->treeView->viewport()->mapToGlobal(pos));
        return;
    }

    // Row-driven menu.
    addAct(tr("Cut"),    m_actCut);
    addAct(tr("Copy"),   m_actCopy);
    addAct(tr("Paste"),  m_actPaste);
    menu.addSeparator();
    addAct(tr("Wrap in Collection"),  m_actWrap);
    addAct(tr("Unwrap Collection"),   m_actUnwrap);
    menu.addSeparator();
    addAct(tr("AND"),    m_actCombAnd);
    addAct(tr("OR"),     m_actCombOr);
    addAct(tr("Negate"), m_actNegate);
    menu.addSeparator();
    addAct(tr("Delete"), m_actDelete);

    menu.exec(ui->treeView->viewport()->mapToGlobal(pos));
}

void TargetingDialog::onSelectionChanged()
{
    const auto items = selectedItems();
    showInlineEditor(items.size() == 1 ? items.first() : nullptr);
    updateActionStates();
}

void TargetingDialog::updateActionStates()
{
    const auto items     = selectedItems();
    const bool any       = !items.isEmpty();
    const bool single    = items.size() == 1;
    const bool siblings  = !selectedContiguousSiblings().isEmpty();
    auto *first          = single ? items.first() : nullptr;
    const bool isColl    = first && first->isCollection();

    const auto *mime = QApplication::clipboard()->mimeData();
    const bool canPaste =
        mime && mime->hasFormat(QString::fromLatin1(kTargetingClipboardMimeType));

    const bool combinatorAllowed = single && first && first->tagRow().row > 0;

    if (m_actDelete)  m_actDelete->setEnabled(any);
    if (m_actCut)     m_actCut->setEnabled(any);
    if (m_actCopy)    m_actCopy->setEnabled(any);
    if (m_actPaste)   m_actPaste->setEnabled(canPaste);
    if (m_actWrap)    m_actWrap->setEnabled(siblings);
    if (m_actUnwrap)  m_actUnwrap->setEnabled(single && isColl);
    if (m_actUp)      m_actUp->setEnabled(single && first->tagRow().row > 0);
    if (m_actDown)
    {
        const bool canDown =
            single && first->parent()
            && first->tagRow().row + 1 < first->parent()->itemCount(first->tagRow().tag);
        m_actDown->setEnabled(canDown);
    }
    if (m_actNegate)
    {
        m_actNegate->setEnabled(single);
        m_actNegate->setChecked(single && first->negated());
    }
    if (m_actCombAnd)
    {
        m_actCombAnd->setEnabled(combinatorAllowed);
        m_actCombAnd->setChecked(combinatorAllowed
                                 && first->combinator() == QLatin1String("AND"));
    }
    if (m_actCombOr)
    {
        m_actCombOr->setEnabled(combinatorAllowed);
        m_actCombOr->setChecked(combinatorAllowed
                                && first->combinator() == QLatin1String("OR"));
    }

    // Item Options compound (toolbar widget).
    if (m_combCombinator)
    {
        QSignalBlocker block(m_combCombinator);
        m_combCombinator->setEnabled(combinatorAllowed);
        if (combinatorAllowed)
        {
            m_combCombinator->setCurrentIndex(
                first->combinator() == QLatin1String("OR") ? 1 : 0);
        }
    }
    if (m_combNegated)
    {
        QSignalBlocker block(m_combNegated);
        m_combNegated->setEnabled(single);
        if (single)
        {
            m_combNegated->setCurrentIndex(first->negated() ? 1 : 0);
        }
    }
}

} // namespace preferences
