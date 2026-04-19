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
#include "ui_targetingdialog.h"

#include "common/basepreferencewidget.h"
#include "targetingfilteritem.h"
#include "targetingicons.h"
#include "targetingmodel.h"
#include "targetingwidgetfactory.h"

#include <mvvm/model/sessionitem.h>
#include <mvvm/model/sessionmodel.h>
#include <mvvm/model/tagrow.h>
#include <mvvm/viewmodel/viewmodel.h>

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDataStream>
#include <QDialogButtonBox>
#include <QHash>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QToolButton>
#include <QUuid>
#include <QVBoxLayout>

namespace preferences
{

namespace
{

constexpr const char *kClipboardMimeType = "application/x-gpui-targeting-filter";

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

} // namespace

TargetingDialog::TargetingDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TargetingDialog())
    , m_model(std::make_unique<TargetingModel>())
    , m_widgetFactory(std::make_unique<TargetingWidgetFactory>())
{
    ui->setupUi(this);
    ui->treeView->setModel(m_model->viewModel());
    ui->treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->treeView->setUniformRowHeights(true);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView, &QTreeView::customContextMenuRequested,
            this, &TargetingDialog::onTreeContextMenu);
    connect(ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, [this](const QItemSelection &, const QItemSelection &) { onSelectionChanged(); });

    buildToolbar();
    updateActionStates();
}

TargetingDialog::~TargetingDialog()
{
    delete ui;
}

void TargetingDialog::setContainer(const TargetingContainer &container)
{
    m_model->fromContainer(container);
    ui->treeView->expandAll();
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

void TargetingDialog::buildToolbar()
{
    auto *newButton = new QToolButton(this);
    newButton->setText(tr("New Item"));
    newButton->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    newButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    newButton->setPopupMode(QToolButton::InstantPopup);
    auto *menu = new QMenu(newButton);
    for (const auto &name : filterCatalog())
    {
        const QString label = name.startsWith(QLatin1String("Filter")) ? name.mid(6) : name;
        auto *action = menu->addAction(TargetingIcons::iconFor(name), label);
        connect(action, &QAction::triggered, this, [this, name]() { addFilter(name); });
    }
    newButton->setMenu(menu);
    ui->toolBar->addWidget(newButton);

    m_actDelete = ui->toolBar->addAction(QIcon::fromTheme(QStringLiteral("edit-delete")),
                                         tr("Delete"));
    connect(m_actDelete, &QAction::triggered, this, &TargetingDialog::deleteSelected);

    ui->toolBar->addSeparator();
    m_actUp   = ui->toolBar->addAction(QIcon::fromTheme(QStringLiteral("go-up")),   tr("Up"));
    m_actDown = ui->toolBar->addAction(QIcon::fromTheme(QStringLiteral("go-down")), tr("Down"));
    connect(m_actUp,   &QAction::triggered, this, &TargetingDialog::moveUp);
    connect(m_actDown, &QAction::triggered, this, &TargetingDialog::moveDown);

    ui->toolBar->addSeparator();
    m_actCut    = ui->toolBar->addAction(QIcon::fromTheme(QStringLiteral("edit-cut")),   tr("Cut"));
    m_actCopy   = ui->toolBar->addAction(QIcon::fromTheme(QStringLiteral("edit-copy")),  tr("Copy"));
    m_actPaste  = ui->toolBar->addAction(QIcon::fromTheme(QStringLiteral("edit-paste")), tr("Paste"));
    connect(m_actCut,   &QAction::triggered, this, &TargetingDialog::cutSelected);
    connect(m_actCopy,  &QAction::triggered, this, &TargetingDialog::copySelected);
    connect(m_actPaste, &QAction::triggered, this, &TargetingDialog::pasteAtSelection);

    ui->toolBar->addSeparator();
    m_actWrap   = ui->toolBar->addAction(QIcon::fromTheme(QStringLiteral("folder-new")),
                                         tr("Wrap in Collection"));
    m_actUnwrap = ui->toolBar->addAction(QIcon::fromTheme(QStringLiteral("edit-clear")),
                                         tr("Unwrap Collection"));
    connect(m_actWrap,   &QAction::triggered, this, &TargetingDialog::wrapInCollection);
    connect(m_actUnwrap, &QAction::triggered, this, &TargetingDialog::unwrapCollection);

    ui->toolBar->addSeparator();
    m_actNegate = ui->toolBar->addAction(QIcon::fromTheme(QStringLiteral("edit-undo")),
                                         tr("Negate"));
    m_actNegate->setCheckable(true);
    connect(m_actNegate, &QAction::triggered, this, &TargetingDialog::toggleNegate);

    m_actCombAnd = ui->toolBar->addAction(tr("AND"));
    m_actCombOr  = ui->toolBar->addAction(tr("OR"));
    m_actCombAnd->setCheckable(true);
    m_actCombOr->setCheckable(true);
    connect(m_actCombAnd, &QAction::triggered, this, &TargetingDialog::setCombinatorAnd);
    connect(m_actCombOr,  &QAction::triggered, this, &TargetingDialog::setCombinatorOr);

    ui->toolBar->addSeparator();
    m_actOptions = ui->toolBar->addAction(QIcon::fromTheme(QStringLiteral("document-properties")),
                                          tr("Options..."));
    connect(m_actOptions, &QAction::triggered, this, &TargetingDialog::editProperties);
}

QList<TargetingFilterItem *> TargetingDialog::selectedItems() const
{
    QList<TargetingFilterItem *> out;
    auto *vm = m_model->viewModel();
    if (!vm)
    {
        return out;
    }
    QSet<TargetingFilterItem *> seen;
    for (const auto &idx : ui->treeView->selectionModel()->selectedRows())
    {
        if (auto *item = dynamic_cast<TargetingFilterItem *>(vm->sessionItemFromIndex(idx)))
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

void TargetingDialog::addFilter(const QString &filterName)
{
    auto *vm   = m_model->viewModel();
    auto items = selectedItems();
    auto *current = items.isEmpty() ? nullptr : items.last();

    ModelView::SessionItem *parent = nullptr;
    int insertRow = -1;

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

    const std::string &tag = childTagFor(parent, m_model->rootFilterParent());

    auto *item = m_model->sessionModel()->insertItem<TargetingFilterItem>(parent, {tag, insertRow});

    TargetingFilterRecord record;
    record.name = filterName;
    record.id   = QUuid::createUuid().toString();
    item->loadRecord(record);

    const auto indices = vm->indexOfSessionItem(item);
    if (!indices.empty())
    {
        ui->treeView->setCurrentIndex(indices.front());
    }
    updateActionStates();
}

void TargetingDialog::deleteSelected()
{
    // MSAD behaviour: when a Collection is removed, its children survive
    // by being re-parented to the deleted node's parent at the original
    // position. Non-collection deletes are straightforward.
    const auto items = selectedItems();

    QList<ModelView::SessionItem *> targets;
    QHash<ModelView::SessionItem *, ModelView::SessionItem *> reparentTargets;
    for (auto *item : items)
    {
        if (!item || !item->parent())
        {
            continue;
        }
        targets.append(item);
    }

    auto *model = m_model->sessionModel();
    auto *root  = m_model->rootFilterParent();

    // Sort deletions deepest-first so re-parenting positions stay stable.
    std::sort(targets.begin(), targets.end(),
              [](ModelView::SessionItem *a, ModelView::SessionItem *b) {
                  return a->tagRow().row > b->tagRow().row;
              });

    for (auto *item : targets)
    {
        auto *parent       = item->parent();
        const auto tagRow  = item->tagRow();
        auto *asCollection = dynamic_cast<TargetingFilterItem *>(item);
        const bool isColl  = asCollection && asCollection->isCollection();

        if (isColl)
        {
            const std::string &dstTag = childTagFor(parent, root);
            int insertAt              = tagRow.row;
            // Move children to parent at the collection's slot, in order.
            while (asCollection->itemCount(TargetingFilterItem::kChildrenTag) > 0)
            {
                model->moveItem(asCollection->getItem(TargetingFilterItem::kChildrenTag, 0),
                                parent, {dstTag, insertAt});
                ++insertAt;
            }
        }

        model->removeItem(parent, item->tagRow());
    }

    updateActionStates();
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
        ui->treeView->setCurrentIndex(indices.front());
    }
    updateActionStates();
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
        ui->treeView->setCurrentIndex(indices.front());
    }
    updateActionStates();
}

void TargetingDialog::cutSelected()
{
    copySelected();
    deleteSelected();
}

void TargetingDialog::copySelected()
{
    const auto items = selectedItems();
    if (items.isEmpty())
    {
        return;
    }

    QList<TargetingFilterRecord> records;
    records.reserve(items.size());
    for (auto *item : items)
    {
        records.append(item->toRecord());
    }

    auto *mime = new QMimeData();
    mime->setData(QString::fromLatin1(kClipboardMimeType), encodeRecords(records));
    QApplication::clipboard()->setMimeData(mime);
}

void TargetingDialog::pasteAtSelection()
{
    const auto *mime = QApplication::clipboard()->mimeData();
    if (!mime || !mime->hasFormat(QString::fromLatin1(kClipboardMimeType)))
    {
        return;
    }

    auto records = decodeRecords(mime->data(QString::fromLatin1(kClipboardMimeType)));
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
            ui->treeView->setCurrentIndex(indices.front());
        }
    }
    ui->treeView->expandAll();
    updateActionStates();
}

void TargetingDialog::wrapInCollection()
{
    const auto siblings = selectedSiblings();
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
        ui->treeView->setCurrentIndex(indices.front());
    }
    ui->treeView->expandAll();
    updateActionStates();
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
    while (collection->itemCount(TargetingFilterItem::kChildrenTag) > 0)
    {
        model->moveItem(collection->getItem(TargetingFilterItem::kChildrenTag, 0),
                        parent, {dst, insertAt});
        ++insertAt;
    }
    model->removeItem(parent, srcTag);

    updateActionStates();
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

void TargetingDialog::editProperties()
{
    const auto items = selectedItems();
    if (items.size() != 1)
    {
        return;
    }
    auto *item = items.first();
    if (item->isCollection())
    {
        return;
    }

    auto editor = m_widgetFactory->create(item->filterName());
    if (!editor)
    {
        QMessageBox::information(this, tr("Targeting Editor"),
                                 tr("No editor is registered for %1.").arg(item->filterName()));
        return;
    }
    editor->setItem(item);

    QDialog dlg(this);
    dlg.setWindowTitle(item->filterName());
    auto *layout = new QVBoxLayout(&dlg);
    auto *raw    = editor.release();
    raw->setParent(&dlg);
    layout->addWidget(raw);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted)
    {
        raw->submit();
    }
}

void TargetingDialog::onTreeContextMenu(const QPoint &pos)
{
    QMenu menu(this);

    auto items     = selectedItems();
    const bool any = !items.isEmpty();

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

    if (any && items.size() == 1 && !items.first()->isCollection())
    {
        addAct(tr("Edit..."), m_actOptions);
        menu.addSeparator();
    }
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
    updateActionStates();
}

void TargetingDialog::updateActionStates()
{
    const auto items     = selectedItems();
    const bool any       = !items.isEmpty();
    const bool single    = items.size() == 1;
    const bool siblings  = !selectedSiblings().isEmpty();
    auto *first          = single ? items.first() : nullptr;
    const bool isColl    = first && first->isCollection();

    const auto *mime = QApplication::clipboard()->mimeData();
    const bool canPaste =
        mime && mime->hasFormat(QString::fromLatin1(kClipboardMimeType));

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
    if (m_actOptions)
    {
        m_actOptions->setEnabled(single && !isColl);
    }
}

} // namespace preferences
