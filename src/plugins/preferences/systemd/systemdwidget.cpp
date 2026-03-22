#include "systemdwidget.h"

#include "ui_systemdwidget.h"

#include <algorithm>
#include <functional>
#include <optional>
#include <sstream>
#include <utility>
#include <vector>
#include <QRegularExpression>

#include "common/commonitem.h"
#include "systemdmancatalog.h"
#include "systemditem.h"
#include "systemdunitsyntaxhighlighter.h"

namespace
{

const char *GENERATED_EDITS_HEADER = "# GPUI-GENERATED: edits-from-table";
constexpr int EDIT_ROLE_SCAFFOLD = Qt::UserRole + 1;
constexpr int EDIT_ROLE_STRICT_MANDATORY = Qt::UserRole + 2;
constexpr int EDIT_ROLE_MANDATORY_GROUP = Qt::UserRole + 3;
constexpr int DEFAULT_DROP_IN_PREFIX = 50;
const char *DEFAULT_DROP_IN_BASE_NAME = "gpo";
const char *DROP_IN_NAME_PATTERN = "^[A-Za-z0-9_.@-]{1,120}$";

QString unitTypeLabel(preferences::SystemdUnitType type)
{
    using preferences::SystemdUnitType;

    switch (type)
    {
    case SystemdUnitType::Service:
        return QObject::tr("Service");
    case SystemdUnitType::Socket:
        return QObject::tr("Socket");
    case SystemdUnitType::Timer:
        return QObject::tr("Timer");
    case SystemdUnitType::Path:
        return QObject::tr("Path");
    case SystemdUnitType::Mount:
        return QObject::tr("Mount");
    case SystemdUnitType::Automount:
        return QObject::tr("Automount");
    case SystemdUnitType::Swap:
        return QObject::tr("Swap");
    case SystemdUnitType::Target:
        return QObject::tr("Target");
    case SystemdUnitType::Device:
        return QObject::tr("Device");
    case SystemdUnitType::Slice:
        return QObject::tr("Slice");
    case SystemdUnitType::Scope:
        return QObject::tr("Scope");
    default:
        return QObject::tr("Unit");
    }
}

QString unitTypeSuffix(preferences::SystemdUnitType type)
{
    using preferences::SystemdUnitType;

    switch (type)
    {
    case SystemdUnitType::Service:
        return ".service";
    case SystemdUnitType::Socket:
        return ".socket";
    case SystemdUnitType::Timer:
        return ".timer";
    case SystemdUnitType::Path:
        return ".path";
    case SystemdUnitType::Mount:
        return ".mount";
    case SystemdUnitType::Automount:
        return ".automount";
    case SystemdUnitType::Swap:
        return ".swap";
    case SystemdUnitType::Target:
        return ".target";
    case SystemdUnitType::Device:
        return ".device";
    case SystemdUnitType::Slice:
        return ".slice";
    case SystemdUnitType::Scope:
        return ".scope";
    default:
        return QString();
    }
}

QComboBox *createStrategyComboBox(QWidget *parent, bool allowReset)
{
    auto *comboBox = new QComboBox(parent);
    comboBox->addItem(QCoreApplication::translate("SystemdWidget", "Replace value"));
    comboBox->addItem(QCoreApplication::translate("SystemdWidget", "Add value"));
    if (allowReset)
    {
        comboBox->addItem(QCoreApplication::translate("SystemdWidget", "Reset key"));
    }
    return comboBox;
}

QString makeEditKey(const QString &section, const QString &key)
{
    return section + QLatin1Char('\n') + key;
}

bool equalsInsensitive(const QString &lhs, const QString &rhs)
{
    return QString::compare(lhs.trimmed(), rhs.trimmed(), Qt::CaseInsensitive) == 0;
}

struct DropInParts
{
    int prefix{DEFAULT_DROP_IN_PREFIX};
    QString baseName{QString::fromLatin1(DEFAULT_DROP_IN_BASE_NAME)};
};

DropInParts splitDropInName(const QString &dropInName)
{
    DropInParts parts;
    const QString trimmed = dropInName.trimmed();
    if (trimmed.isEmpty())
    {
        return parts;
    }

    static const QRegularExpression fullPattern(QStringLiteral("^(\\d{1,3})-([A-Za-z0-9_.@-]{1,120})\\.conf$"));
    const auto fullMatch = fullPattern.match(trimmed);
    if (fullMatch.hasMatch())
    {
        parts.prefix = fullMatch.captured(1).toInt();
        parts.baseName = fullMatch.captured(2);
        return parts;
    }

    QString normalized = trimmed;
    if (normalized.endsWith(QStringLiteral(".conf"), Qt::CaseInsensitive))
    {
        normalized.chop(5);
    }

    static const QRegularExpression legacyPattern(QStringLiteral("^(\\d{1,3})-(.+)$"));
    const auto legacyMatch = legacyPattern.match(normalized);
    if (legacyMatch.hasMatch())
    {
        parts.prefix = legacyMatch.captured(1).toInt();
        parts.baseName = legacyMatch.captured(2);
    }
    else if (!normalized.isEmpty())
    {
        parts.baseName = normalized;
    }

    return parts;
}

class ActionAutocompleteDelegate : public QStyledItemDelegate
{
public:
    using Provider = std::function<QStringList(int)>;
    using Normalizer = std::function<std::optional<QString>(int, const QString &)>;

    explicit ActionAutocompleteDelegate(Provider provider, Normalizer normalizer = Normalizer(), QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
        , m_provider(std::move(provider))
        , m_normalizer(std::move(normalizer))
    {}

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override
    {
        Q_UNUSED(option)
        auto *combo = new QComboBox(parent);
        combo->setEditable(true);
        combo->setInsertPolicy(QComboBox::NoInsert);
        combo->addItems(m_provider ? m_provider(index.row()) : QStringList());
        combo->setMaxVisibleItems(20);

        if (auto *completer = combo->completer())
        {
            completer->setCaseSensitivity(Qt::CaseInsensitive);
            completer->setFilterMode(Qt::MatchContains);
            completer->setCompletionMode(QCompleter::PopupCompletion);
        }

        const QFontMetrics fm(combo->font());
        int popupWidth = 0;
        for (int i = 0; i < combo->count(); ++i)
        {
            popupWidth = std::max(popupWidth, fm.horizontalAdvance(combo->itemText(i)));
        }
        popupWidth += combo->style()->pixelMetric(QStyle::PM_ScrollBarExtent)
                    + combo->style()->pixelMetric(QStyle::PM_FocusFrameHMargin) * 4;
        if (auto *view = combo->view())
        {
            view->setMinimumWidth(std::max(view->minimumWidth(), popupWidth));
        }

        return combo;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override
    {
        auto *combo = qobject_cast<QComboBox *>(editor);
        if (!combo)
        {
            return;
        }

        const QString value = index.data(Qt::EditRole).toString();
        combo->setEditText(value);
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override
    {
        auto *combo = qobject_cast<QComboBox *>(editor);
        if (!combo)
        {
            return;
        }

        QString value = combo->currentText().trimmed();
        if (m_normalizer)
        {
            const auto normalized = m_normalizer(index.row(), value);
            if (!normalized.has_value())
            {
                return;
            }
            value = normalized.value();
        }
        model->setData(index, value, Qt::EditRole);
    }

private:
    Provider m_provider;
    Normalizer m_normalizer;
};

QString stripInlineComment(const QString &value)
{
    bool inSingleQuote = false;
    bool inDoubleQuote = false;
    bool escaped = false;

    for (int i = 0; i < value.size(); ++i)
    {
        const QChar ch = value.at(i);
        if (escaped)
        {
            escaped = false;
            continue;
        }

        if (ch == QLatin1Char('\\'))
        {
            escaped = true;
            continue;
        }

        if (!inDoubleQuote && ch == QLatin1Char('\''))
        {
            inSingleQuote = !inSingleQuote;
            continue;
        }

        if (!inSingleQuote && ch == QLatin1Char('"'))
        {
            inDoubleQuote = !inDoubleQuote;
            continue;
        }

        if (!inSingleQuote && !inDoubleQuote
            && (ch == QLatin1Char('#') || ch == QLatin1Char(';'))
            && (i == 0 || value.at(i - 1).isSpace()))
        {
            return value.left(i).trimmed();
        }
    }

    return value.trimmed();
}

QList<int> selectedRows(QTableWidget *table)
{
    QList<int> rows;
    if (!table || !table->selectionModel())
    {
        return rows;
    }

    const auto selectedModelRows = table->selectionModel()->selectedRows();
    for (const auto &index : selectedModelRows)
    {
        rows.push_back(index.row());
    }

    if (rows.isEmpty())
    {
        const int current = table->currentRow();
        if (current >= 0)
        {
            rows.push_back(current);
        }
    }

    std::sort(rows.begin(), rows.end());
    rows.erase(std::unique(rows.begin(), rows.end()), rows.end());
    return rows;
}

void selectRows(QTableWidget *table, const QList<int> &rows)
{
    if (!table || !table->selectionModel())
    {
        return;
    }

    table->clearSelection();
    for (const int row : rows)
    {
        if (row >= 0 && row < table->rowCount())
        {
            const auto index = table->model()->index(row, 0);
            table->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }
    }
}

void closePersistentEditors(QTableWidget *table)
{
    if (!table)
    {
        return;
    }

    for (int row = 0; row < table->rowCount(); ++row)
    {
        for (int col = 0; col < table->columnCount(); ++col)
        {
            if (auto *item = table->item(row, col))
            {
                table->closePersistentEditor(item);
            }
        }
    }
}

QSet<int> moveRowIndexes(const QList<int> &rows, int rowCount, bool moveUp, const std::function<void(int, int)> &swapper)
{
    QSet<int> selectedSet;
    for (const int row : rows)
    {
        selectedSet.insert(row);
    }

    if (moveUp)
    {
        for (const int row : rows)
        {
            if (row == 0 || selectedSet.contains(row - 1))
            {
                continue;
            }

            swapper(row, row - 1);
            selectedSet.remove(row);
            selectedSet.insert(row - 1);
        }
    }
    else
    {
        for (int i = rows.size() - 1; i >= 0; --i)
        {
            const int row = rows.at(i);
            if (row == rowCount - 1 || selectedSet.contains(row + 1))
            {
                continue;
            }

            swapper(row, row + 1);
            selectedSet.remove(row);
            selectedSet.insert(row + 1);
        }
    }

    return selectedSet;
}

} // namespace

namespace preferences
{

SystemdWidget::SystemdWidget(QWidget *parent, SystemdItem *item)
    : BasePreferenceWidget(parent)
    , m_item(item)
    , ui(new Ui::SystemdWidget())
    , m_preferredFlexibleEditMode(static_cast<int>(SystemdEditMode::Override))
{
    ui->setupUi(this);
    ui->dropInBaseNameLineEdit->setValidator(new QRegularExpressionValidator(
        QRegularExpression(QString::fromLatin1(DROP_IN_NAME_PATTERN)),
        ui->dropInBaseNameLineEdit));
    ui->actionsTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->actionsTableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
    ui->actionsTableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
    ui->actionsTableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    ui->dependenciesTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->dependenciesTableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    (void)new SystemdUnitSyntaxHighlighter(ui->unitFileTextEdit->document());
    applyActionTableDelegates();

    connect(ui->addActionPushButton, SIGNAL(clicked()), this, SLOT(on_actionAddButton_clicked()));
    connect(ui->clearActionsPushButton, SIGNAL(clicked()), this, SLOT(on_actionsClearButton_clicked()));
    connect(ui->removeActionPushButton, SIGNAL(clicked()), this, SLOT(on_actionRemoveButton_clicked()));

    connect(ui->addDependPushButton, SIGNAL(clicked()), this, SLOT(on_dependAddButton_clicked()));
    connect(ui->clearDependsPushButton, SIGNAL(clicked()), this, SLOT(on_dependsClearButton_clicked()));
    connect(ui->removeDependPushButton, SIGNAL(clicked()), this, SLOT(on_dependRemoveButton_clicked()));

    connect(ui->changeUnitStateCheckBox,
            &QCheckBox::toggled,
            this,
            [this](bool) { updateStateControls(); });
    connect(ui->editUnitFileCheckBox,
            &QCheckBox::toggled,
            this,
            [this](bool) { updateEditControls(); });
    connect(ui->applyModeComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            [this](int) {
                updateForcedOptions();
                updateStateControls();
                updateEditModeAvailability();
                updateEditControls();
            });
    connect(ui->policyTargetComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            [this](int) {
                updatePolicyTargetControls();
                updateForcedOptions();
                updateStateControls();
            });
    connect(ui->unitEditModeComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            [this](int index) {
                if (allowedEditModesForApplyMode().size() > 1)
                {
                    m_preferredFlexibleEditMode = index;
                }
                updateEditModeAvailability();
            });
}

SystemdWidget::~SystemdWidget()
{
    delete ui;
}

void SystemdWidget::setItem(ModelView::SessionItem *item)
{
    m_item = dynamic_cast<SystemdItem *>(item);
    if (!m_item)
    {
        return;
    }

    ui->unitNameLineEdit->setText(QString::fromStdString(m_item->property<std::string>(SystemdItem::UNIT)));

    ui->changeUnitStateCheckBox->setChecked(m_item->property<int>(SystemdItem::STATE) != static_cast<int>(SystemdState::AsIs));
    ui->changeUnitStateComboBox->setCurrentIndex(
        std::max(0, m_item->property<int>(SystemdItem::STATE) - static_cast<int>(SystemdState::Enable)));
    ui->nowCheckBox->setChecked(m_item->property<bool>(SystemdItem::STATE_NOW));

    ui->applyModeComboBox->setCurrentIndex(m_item->property<int>(SystemdItem::APPLY_MODE));
    ui->policyTargetComboBox->setCurrentIndex(m_item->property<int>(SystemdItem::POLICY_TARGET));

    ui->editUnitFileCheckBox->setChecked(m_item->property<bool>(SystemdItem::EDIT));
    ui->unitEditModeComboBox->setCurrentIndex(m_item->property<int>(SystemdItem::EDIT_MODE));
    if (allowedEditModesForApplyMode().size() > 1)
    {
        m_preferredFlexibleEditMode = ui->unitEditModeComboBox->currentIndex();
    }
    setDropInName(QString::fromStdString(m_item->property<std::string>(SystemdItem::DROP_IN_NAME)));
    ui->unitFileTextEdit->setPlainText(QString::fromStdString(m_item->property<std::string>(SystemdItem::UNIT_FILE_TEXT)));
    m_textEditorMode = m_item->property<int>(SystemdItem::UNIT_FILE_MODE) == static_cast<int>(SystemdUnitFileMode::Text);

    readEditTable();

    readDependencyTable();

    updateUiForUnitType();
    updatePolicyTargetControls();
    updateForcedOptions();
    updateStateControls();
    updateEditModeAvailability();
    updateEditControls();
    updateEditorModeUi();
    updateDependencyControls();
}

void SystemdWidget::readEditTable()
{
    ui->actionsTableWidget->setRowCount(0);

    if (!m_item)
    {
        return;
    }

    for (const auto *edit : m_item->editItems())
    {
        appendActionRow(QString::fromStdString(edit->property<std::string>(SystemdEditItem::SECTION)),
                        QString::fromStdString(edit->property<std::string>(SystemdEditItem::KEY)),
                        QString::fromStdString(edit->property<std::string>(SystemdEditItem::VALUE)),
                        static_cast<SystemdConflictStrategy>(edit->property<int>(SystemdEditItem::STRATEGY)),
                        false,
                        false,
                        QString());
    }
}

void SystemdWidget::writeEditTable()
{
    if (!m_item)
    {
        return;
    }

    auto *table = ui->actionsTableWidget;
    const auto editMode = currentEditMode();
    QList<int> persistedRows;
    for (int row = 0; row < table->rowCount(); ++row)
    {
        const QString section = sectionTextAtRow(row);
        const QString key = keyTextAtRow(row);
        const QString value = valueTextAtRow(row);
        auto *strategyComboBox = qobject_cast<QComboBox *>(table->cellWidget(row, 0));
        const int strategy = editMode == SystemdEditMode::Create
                           ? static_cast<int>(SystemdConflictStrategy::AddValue)
                           : (strategyComboBox ? strategyComboBox->currentIndex()
                                               : static_cast<int>(SystemdConflictStrategy::ReplaceValue));

        if (section.isEmpty() || key.isEmpty())
        {
            continue;
        }

        if (isScaffoldRow(row) && !isStrictMandatoryRow(row) && !scaffoldGroupId(row).isEmpty()
            && strategy != static_cast<int>(SystemdConflictStrategy::ResetKey)
            && value.trimmed().isEmpty())
        {
            continue;
        }

        persistedRows.push_back(row);
    }

    m_item->editLength(persistedRows.size());
    const auto items = m_item->editItems();
    for (int i = 0; i < persistedRows.size(); ++i)
    {
        const int row = persistedRows.at(i);
        auto *strategyComboBox = qobject_cast<QComboBox *>(table->cellWidget(row, 0));
        const int strategy = editMode == SystemdEditMode::Create
                           ? static_cast<int>(SystemdConflictStrategy::AddValue)
                           : (strategyComboBox ? strategyComboBox->currentIndex()
                                               : static_cast<int>(SystemdConflictStrategy::ReplaceValue));

        items.at(i)->setProperty(SystemdEditItem::SECTION, sectionTextAtRow(row).toStdString());
        items.at(i)->setProperty(SystemdEditItem::KEY, keyTextAtRow(row).toStdString());
        items.at(i)->setProperty(SystemdEditItem::VALUE, valueTextAtRow(row).toStdString());
        items.at(i)->setProperty(SystemdEditItem::STRATEGY, strategy);
    }
}

void SystemdWidget::readDependencyTable()
{
    ui->dependenciesTableWidget->setRowCount(0);

    if (!m_item)
    {
        return;
    }

    for (const auto *dep : m_item->depItems())
    {
        const int row = ui->dependenciesTableWidget->rowCount();
        ui->dependenciesTableWidget->insertRow(row);
        attachDependencyTypeComboBox(row, dep->property<int>(SystemdDependencyItem::TYPE));
        ui->dependenciesTableWidget->setItem(
            row,
            1,
            new QTableWidgetItem(QString::fromStdString(dep->property<std::string>(SystemdDependencyItem::PATH))));
    }
}

void SystemdWidget::writeDependencyTable()
{
    if (!m_item)
    {
        return;
    }

    auto *table = ui->dependenciesTableWidget;
    m_item->depLength(table->rowCount());

    const auto items = m_item->depItems();
    for (int i = 0; i < static_cast<int>(items.size()); ++i)
    {
        auto *combo = qobject_cast<QComboBox *>(table->cellWidget(i, 0));
        const auto *pathWidget = table->item(i, 1);

        items.at(i)->setProperty(SystemdDependencyItem::TYPE, combo ? combo->currentIndex() : 0);
        items.at(i)->setProperty(SystemdDependencyItem::PATH,
                                 pathWidget ? pathWidget->text().toStdString() : std::string());
    }
}

void SystemdWidget::updateUiForUnitType()
{
    if (!m_item)
    {
        return;
    }

    const auto type = static_cast<SystemdUnitType>(m_item->property<int>(SystemdItem::UNIT_TYPE));

    ui->unitGroupBox->setTitle(unitTypeLabel(type));
    ui->unitTypeLabel->setText(unitTypeSuffix(type));

    const bool hasDependencies = m_item->property<bool>(SystemdItem::HAS_DEPENDENCIES);
    ui->dependGroupBox->setVisible(hasDependencies);
    updateActionColumnsMinimumWidths();
}

void SystemdWidget::updateStateControls()
{
    const bool enabled = ui->changeUnitStateCheckBox->isChecked();
    ui->changeUnitStateComboBox->setEnabled(enabled);

    const bool nowSupported = enabled && !isMachineGlobalUserTarget();
    if (!nowSupported)
    {
        ui->nowCheckBox->setChecked(false);
    }
    ui->nowCheckBox->setEnabled(nowSupported);

    const QString globalNowHint = QCoreApplication::translate(
        "SystemdWidget",
        "For computer policies targeting users (--global), immediate start is unavailable. "
        "The policy only changes default user unit files and cannot start user units right away.");
    const QString regularNowHint = QCoreApplication::translate(
        "SystemdWidget",
        "Apply the state immediately after the policy is processed.");
    const QString nowHint = isMachineGlobalUserTarget() ? globalNowHint : regularNowHint;
    ui->nowCheckBox->setToolTip(nowHint);
    ui->nowCheckBox->setWhatsThis(nowHint);
}

void SystemdWidget::updatePolicyTargetControls()
{
    if (!m_item)
    {
        return;
    }

    const bool userContextPolicy = isUserPolicyContext();
    ui->policyTargetComboBox->setVisible(!userContextPolicy);
    ui->policyTargetLabel->setVisible(!userContextPolicy);
    if (userContextPolicy)
    {
        ui->policyTargetComboBox->setCurrentIndex(static_cast<int>(SystemdPolicyTarget::User));
        ui->policyTargetHintLabel->setText(QCoreApplication::translate("SystemdWidget", "Applies to user units."));
        ui->policyTargetHintLabel->setVisible(true);
    }
    else
    {
        const bool globalUserTarget = static_cast<SystemdPolicyTarget>(ui->policyTargetComboBox->currentIndex())
            == SystemdPolicyTarget::User;
        if (globalUserTarget)
        {
            const QString hint = QCoreApplication::translate(
                "SystemdWidget",
                "Applies to global user units in /etc/systemd/user. "
                "Immediate start (--now) is unavailable for this target.");
            ui->policyTargetHintLabel->setText(hint);
            ui->policyTargetHintLabel->setToolTip(hint);
            ui->policyTargetHintLabel->setVisible(true);
        }
        else
        {
            ui->policyTargetHintLabel->clear();
            ui->policyTargetHintLabel->setToolTip(QString());
            ui->policyTargetHintLabel->setVisible(false);
        }
    }
}

void SystemdWidget::updateEditControls()
{
    updateForcedOptions();
    const bool enabled = ui->editUnitFileCheckBox->isChecked();

    const bool dropInEnabled = enabled && currentEditMode() != SystemdEditMode::Create;
    ui->dropInPrefixSpinBox->setEnabled(dropInEnabled);
    ui->dropInBaseNameLineEdit->setEnabled(dropInEnabled);
    ui->actionsTableWidget->setEnabled(enabled);
    ui->addActionPushButton->setEnabled(enabled);
    ui->removeActionPushButton->setEnabled(enabled);
    ui->clearActionsPushButton->setEnabled(enabled);
    ui->actionMoveUpButton->setEnabled(enabled);
    ui->actionMoveDownButton->setEnabled(enabled);
    ui->unitFileTextEdit->setEnabled(enabled);
    ui->switchEditorModePushButton->setEnabled(enabled);
    ui->dropInLabel->setVisible(enabled);
    ui->editModeHintLabel->setVisible(false);
    updateEditModeAvailability();
    updateEditorModeUi();
}

void SystemdWidget::updateEditModeAvailability()
{
    const QList<int> allowedModes = allowedEditModesForApplyMode();

    for (int i = 0; i < ui->unitEditModeComboBox->count(); ++i)
    {
        const bool allowed = allowedModes.contains(i);
        ui->unitEditModeComboBox->setItemData(i, allowed ? QVariant() : 0, Qt::UserRole - 1);
    }

    ensureValidEditModeSelection();
    ensureScaffoldRows();
    const bool editEnabled = ui->editUnitFileCheckBox->isChecked();
    const bool singleMode = allowedModes.size() == 1;
    ui->editModeLabel->setVisible(editEnabled && !singleMode);
    ui->unitEditModeComboBox->setVisible(editEnabled && !singleMode);
    const bool showHint = editEnabled && singleMode;
    ui->editModeHintLabel->setVisible(showHint);
    ui->editModeHintLabel->setText(showHint ? editModeHintForCurrentSelection() : QString());

    const auto mode = static_cast<SystemdEditMode>(ui->unitEditModeComboBox->currentIndex());
    const bool showDropIn = editEnabled && mode != SystemdEditMode::Create;
    ui->dropInLabel->setVisible(showDropIn);
    ui->dropInPrefixSpinBox->setVisible(showDropIn);
    ui->dropInSeparatorLabel->setVisible(showDropIn);
    ui->dropInBaseNameLineEdit->setVisible(showDropIn);
    ui->dropInSuffixLabel->setVisible(showDropIn);

    const bool showStrategy = editEnabled && mode != SystemdEditMode::Create && !m_textEditorMode;
    ui->actionsTableWidget->setColumnHidden(0, !showStrategy);
}

void SystemdWidget::updateEditorModeUi()
{
    const bool editEnabled = ui->editUnitFileCheckBox->isChecked();
    ui->actionsTableWidget->setVisible(!m_textEditorMode);
    ui->unitFileTextEdit->setVisible(m_textEditorMode);
    ui->clearActionsPushButton->setVisible(!m_textEditorMode);
    ui->removeActionPushButton->setVisible(!m_textEditorMode);
    ui->addActionPushButton->setVisible(!m_textEditorMode);
    ui->actionMoveUpButton->setVisible(!m_textEditorMode);
    ui->actionMoveDownButton->setVisible(!m_textEditorMode);
    ui->switchEditorModePushButton->setVisible(editEnabled);
    ui->switchEditorModePushButton->setText(
        m_textEditorMode
            ? QCoreApplication::translate("SystemdWidget", "Switch to table mode")
            : QCoreApplication::translate("SystemdWidget", "Switch to text mode"));
}

void SystemdWidget::updateActionColumnsMinimumWidths()
{
    auto *table = ui->actionsTableWidget;
    if (!table)
    {
        return;
    }

    const auto type = currentUnitType();
    const auto sections = SystemdManCatalog::sectionsForUnitType(type);
    const QFontMetrics fm(table->font());

    int sectionWidth = 0;
    for (const auto &section : sections)
    {
        sectionWidth = std::max(sectionWidth, fm.horizontalAdvance(section));
    }

    int keyWidth = 0;
    for (const auto &section : sections)
    {
        const auto keys = SystemdManCatalog::keysFor(type, section);
        for (const auto &key : keys)
        {
            keyWidth = std::max(keyWidth, fm.horizontalAdvance(key));
        }
    }

    const auto profile = SystemdManCatalog::mandatoryProfile(type);
    for (const auto &group : profile.oneOfGroups)
    {
        for (const auto &key : group.keys)
        {
            keyWidth = std::max(keyWidth, fm.horizontalAdvance(key));
        }
    }

    const int extraPadding = table->style()->pixelMetric(QStyle::PM_FocusFrameHMargin) * 6 + 24;
    sectionWidth += extraPadding;
    keyWidth += extraPadding;

    if (sectionWidth > 0)
    {
        table->horizontalHeader()->resizeSection(1, std::max(table->columnWidth(1), sectionWidth));
    }
    if (keyWidth > 0)
    {
        table->horizontalHeader()->resizeSection(2, std::max(table->columnWidth(2), keyWidth));
    }
}

bool SystemdWidget::isUserPolicyContext() const
{
    if (!m_item || !m_item->parent())
    {
        return false;
    }

    const auto parentChildren = m_item->parent()->children();
    if (parentChildren.size() < 2)
    {
        return false;
    }

    const auto userContextProperty = CommonItem::propertyToString(CommonItem::USER_CONTEXT);
    return parentChildren[parentChildren.size() - 2]->property<bool>(userContextProperty);
}

void SystemdWidget::updateForcedOptions()
{
    const bool forceEdit = isEditForcedByApplyMode();
    if (forceEdit)
    {
        ui->editUnitFileCheckBox->setChecked(true);
    }
    ui->editUnitFileCheckBox->setEnabled(!forceEdit);

    const QString editHint = forceEdit
        ? QCoreApplication::translate(
              "SystemdWidget",
              "Configuration editing is required for this apply mode because the unit may need to be created.")
        : QString();
    ui->editUnitFileCheckBox->setToolTip(editHint);
    ui->editUnitFileCheckBox->setWhatsThis(editHint);
}

QList<int> SystemdWidget::allowedEditModesForApplyMode() const
{
    const auto applyMode = static_cast<SystemdApplyMode>(ui->applyModeComboBox->currentIndex());
    switch (applyMode)
    {
    case SystemdApplyMode::IfExists:
        return {static_cast<int>(SystemdEditMode::Override)};
    case SystemdApplyMode::IfMissing:
        return {static_cast<int>(SystemdEditMode::Create)};
    case SystemdApplyMode::Always:
    default:
        return {static_cast<int>(SystemdEditMode::Create),
                static_cast<int>(SystemdEditMode::Override),
                static_cast<int>(SystemdEditMode::CreateOrOverride)};
    }
}

void SystemdWidget::ensureValidEditModeSelection()
{
    const QList<int> allowedModes = allowedEditModesForApplyMode();
    if (allowedModes.isEmpty())
    {
        return;
    }

    int desiredMode = ui->unitEditModeComboBox->currentIndex();
    if (allowedModes.size() > 1)
    {
        if (allowedModes.contains(m_preferredFlexibleEditMode))
        {
            desiredMode = m_preferredFlexibleEditMode;
        }
        else if (allowedModes.contains(static_cast<int>(SystemdEditMode::Override)))
        {
            desiredMode = static_cast<int>(SystemdEditMode::Override);
        }
        else
        {
            desiredMode = allowedModes.first();
        }
    }
    else
    {
        desiredMode = allowedModes.first();
    }

    if (ui->unitEditModeComboBox->currentIndex() != desiredMode)
    {
        ui->unitEditModeComboBox->setCurrentIndex(desiredMode);
    }
}

QString SystemdWidget::editModeHintForCurrentSelection() const
{
    const auto applyMode = static_cast<SystemdApplyMode>(ui->applyModeComboBox->currentIndex());
    switch (applyMode)
    {
    case SystemdApplyMode::IfExists:
        return QCoreApplication::translate("SystemdWidget", "Unit will be overridden using a drop-in.");
    case SystemdApplyMode::IfMissing:
        return QCoreApplication::translate("SystemdWidget", "Unit will be created.");
    case SystemdApplyMode::Always:
    default:
        return QString();
    }
}

SystemdUnitType SystemdWidget::currentUnitType() const
{
    if (!m_item)
    {
        return SystemdUnitType::Service;
    }

    return static_cast<SystemdUnitType>(m_item->property<int>(SystemdItem::UNIT_TYPE));
}

SystemdEditMode SystemdWidget::currentEditMode() const
{
    return static_cast<SystemdEditMode>(ui->unitEditModeComboBox->currentIndex());
}

bool SystemdWidget::isMachineGlobalUserTarget() const
{
    return !isUserPolicyContext()
        && static_cast<SystemdPolicyTarget>(ui->policyTargetComboBox->currentIndex()) == SystemdPolicyTarget::User;
}

bool SystemdWidget::isEditForcedByApplyMode() const
{
    const auto applyMode = static_cast<SystemdApplyMode>(ui->applyModeComboBox->currentIndex());
    return applyMode == SystemdApplyMode::Always || applyMode == SystemdApplyMode::IfMissing;
}

QString SystemdWidget::currentDropInName() const
{
    return QStringLiteral("%1-%2.conf")
        .arg(ui->dropInPrefixSpinBox->value())
        .arg(ui->dropInBaseNameLineEdit->text().trimmed());
}

void SystemdWidget::setDropInName(const QString &dropInName)
{
    const auto parts = splitDropInName(dropInName);
    ui->dropInPrefixSpinBox->setValue(parts.prefix);
    ui->dropInBaseNameLineEdit->setText(parts.baseName);
}

bool SystemdWidget::mandatoryRulesActive() const
{
    if (m_textEditorMode)
    {
        return false;
    }

    const auto mode = currentEditMode();
    return mode == SystemdEditMode::Create || mode == SystemdEditMode::CreateOrOverride;
}

void SystemdWidget::ensureActionRowItems(int row)
{
    if (row < 0 || row >= ui->actionsTableWidget->rowCount())
    {
        return;
    }

    if (!ui->actionsTableWidget->item(row, 1))
    {
        ui->actionsTableWidget->setItem(row, 1, new QTableWidgetItem());
    }
    if (!ui->actionsTableWidget->item(row, 2))
    {
        ui->actionsTableWidget->setItem(row, 2, new QTableWidgetItem());
    }
    if (!ui->actionsTableWidget->item(row, 3))
    {
        ui->actionsTableWidget->setItem(row, 3, new QTableWidgetItem());
    }
}

void SystemdWidget::setActionRowMetadata(int row, bool scaffold, bool strictMandatory, const QString &groupId)
{
    ensureActionRowItems(row);

    auto *sectionItem = ui->actionsTableWidget->item(row, 1);
    auto *keyItem = ui->actionsTableWidget->item(row, 2);
    if (!sectionItem || !keyItem)
    {
        return;
    }

    const bool oneOfScaffold = scaffold && !strictMandatory && !groupId.isEmpty();
    Qt::ItemFlags sectionFlags = sectionItem->flags();
    Qt::ItemFlags keyFlags = keyItem->flags();
    sectionFlags |= Qt::ItemIsEditable;
    keyFlags |= Qt::ItemIsEditable;

    if (!scaffold)
    {
        // No-op: both fields remain editable for regular rows.
    }
    else if (strictMandatory)
    {
        sectionFlags &= ~Qt::ItemIsEditable;
        keyFlags &= ~Qt::ItemIsEditable;
    }
    else if (oneOfScaffold)
    {
        sectionFlags &= ~Qt::ItemIsEditable;
    }
    else
    {
        sectionFlags &= ~Qt::ItemIsEditable;
        keyFlags &= ~Qt::ItemIsEditable;
    }

    sectionItem->setFlags(sectionFlags);
    keyItem->setFlags(keyFlags);

    keyItem->setData(EDIT_ROLE_SCAFFOLD, scaffold);
    keyItem->setData(EDIT_ROLE_STRICT_MANDATORY, strictMandatory);
    keyItem->setData(EDIT_ROLE_MANDATORY_GROUP, groupId);
}

bool SystemdWidget::isScaffoldRow(int row) const
{
    const auto *keyItem = ui->actionsTableWidget->item(row, 2);
    return keyItem && keyItem->data(EDIT_ROLE_SCAFFOLD).toBool();
}

bool SystemdWidget::isStrictMandatoryRow(int row) const
{
    const auto *keyItem = ui->actionsTableWidget->item(row, 2);
    return keyItem && keyItem->data(EDIT_ROLE_STRICT_MANDATORY).toBool();
}

QString SystemdWidget::scaffoldGroupId(int row) const
{
    const auto *keyItem = ui->actionsTableWidget->item(row, 2);
    return keyItem ? keyItem->data(EDIT_ROLE_MANDATORY_GROUP).toString() : QString();
}

QString SystemdWidget::sectionTextAtRow(int row) const
{
    const auto *item = ui->actionsTableWidget->item(row, 1);
    return item ? item->text().trimmed() : QString();
}

QString SystemdWidget::keyTextAtRow(int row) const
{
    const auto *item = ui->actionsTableWidget->item(row, 2);
    return item ? item->text().trimmed() : QString();
}

QString SystemdWidget::valueTextAtRow(int row) const
{
    const auto *item = ui->actionsTableWidget->item(row, 3);
    return item ? item->text() : QString();
}

QStringList SystemdWidget::oneOfGroupKeys(const QString &groupId) const
{
    if (groupId.isEmpty())
    {
        return {};
    }

    const auto profile = SystemdManCatalog::mandatoryProfile(currentUnitType());
    const auto groupIt = std::find_if(profile.oneOfGroups.cbegin(),
                                      profile.oneOfGroups.cend(),
                                      [&groupId](const auto &group) {
                                          return group.id == groupId;
                                      });
    return groupIt != profile.oneOfGroups.cend() ? groupIt->keys : QStringList();
}

QStringList SystemdWidget::keySuggestionsForRow(int row) const
{
    const auto groupId = scaffoldGroupId(row);
    if (!groupId.isEmpty())
    {
        const auto groupKeys = oneOfGroupKeys(groupId);
        if (!groupKeys.isEmpty())
        {
            return groupKeys;
        }
    }

    return SystemdManCatalog::keysFor(currentUnitType(), sectionTextAtRow(row));
}

bool SystemdWidget::normalizeKeyForRow(int row, const QString &candidateKey, QString &normalizedKey) const
{
    const QString trimmed = candidateKey.trimmed();
    const auto groupId = scaffoldGroupId(row);
    if (groupId.isEmpty())
    {
        normalizedKey = trimmed;
        return true;
    }

    const auto groupKeys = oneOfGroupKeys(groupId);
    const auto keyIt = std::find_if(groupKeys.cbegin(), groupKeys.cend(), [&trimmed](const auto &groupKey) {
        return equalsInsensitive(trimmed, groupKey);
    });
    if (keyIt == groupKeys.cend())
    {
        return false;
    }

    normalizedKey = *keyIt;
    return true;
}

int SystemdWidget::appendActionRow(const QString &section,
                                   const QString &key,
                                   const QString &value,
                                   SystemdConflictStrategy strategy,
                                   bool scaffold,
                                   bool strictMandatory,
                                   const QString &groupId)
{
    const int row = ui->actionsTableWidget->rowCount();
    ui->actionsTableWidget->insertRow(row);
    ensureActionRowItems(row);
    ui->actionsTableWidget->item(row, 1)->setText(section);
    ui->actionsTableWidget->item(row, 2)->setText(key);
    ui->actionsTableWidget->item(row, 3)->setText(value);
    setActionRowMetadata(row, scaffold, strictMandatory, groupId);
    attachStrategyComboBox(row);
    if (auto *combo = qobject_cast<QComboBox *>(ui->actionsTableWidget->cellWidget(row, 0)))
    {
        int strategyIndex = static_cast<int>(strategy);
        if (combo->count() < 3 && strategy == SystemdConflictStrategy::ResetKey)
        {
            strategyIndex = static_cast<int>(SystemdConflictStrategy::ReplaceValue);
        }
        combo->setCurrentIndex(std::clamp(strategyIndex, 0, combo->count() - 1));
    }
    applyStrategyStateToRow(row);
    return row;
}

void SystemdWidget::ensureScaffoldRows()
{
    if (m_textEditorMode)
    {
        return;
    }

    const bool active = mandatoryRulesActive();
    if (!active)
    {
        for (int row = 0; row < ui->actionsTableWidget->rowCount(); ++row)
        {
            setActionRowMetadata(row, false, false, QString());
        }
        refreshStrategyForAllRows();
        return;
    }

    const auto type = currentUnitType();
    const auto profile = SystemdManCatalog::mandatoryProfile(type);

    for (int row = 0; row < ui->actionsTableWidget->rowCount(); ++row)
    {
        setActionRowMetadata(row, false, false, QString());
    }

    auto findFirstExact = [this](const QString &section, const QString &key) -> int {
        for (int row = 0; row < ui->actionsTableWidget->rowCount(); ++row)
        {
            if (equalsInsensitive(sectionTextAtRow(row), section) && equalsInsensitive(keyTextAtRow(row), key))
            {
                return row;
            }
        }
        return -1;
    };

    auto rowsInGroup = [this](const MandatoryGroup &group) {
        QList<int> rows;
        for (int row = 0; row < ui->actionsTableWidget->rowCount(); ++row)
        {
            if (!equalsInsensitive(sectionTextAtRow(row), group.section))
            {
                continue;
            }
            const QString rowKey = keyTextAtRow(row);
            const auto keyIt = std::find_if(group.keys.cbegin(), group.keys.cend(), [&rowKey](const auto &groupKey) {
                return equalsInsensitive(rowKey, groupKey);
            });
            if (keyIt != group.keys.cend())
            {
                rows.push_back(row);
            }
        }
        return rows;
    };

    for (const auto &strict : profile.strictKeys)
    {
        int row = findFirstExact(strict.first, strict.second);
        if (row < 0)
        {
            row = appendActionRow(strict.first,
                                  strict.second,
                                  QString(),
                                  SystemdConflictStrategy::ReplaceValue,
                                  true,
                                  true,
                                  QString());
        }
        setActionRowMetadata(row, true, true, QString());
    }

    for (const auto &group : profile.oneOfGroups)
    {
        int scaffoldRow = !group.defaultKey.isEmpty() ? findFirstExact(group.section, group.defaultKey) : -1;
        const auto existingRows = rowsInGroup(group);
        if (scaffoldRow < 0 && !existingRows.isEmpty())
        {
            scaffoldRow = existingRows.first();
        }
        if (scaffoldRow < 0 && existingRows.isEmpty())
        {
            const QString defaultKey = !group.defaultKey.isEmpty() ? group.defaultKey : group.keys.value(0);
            scaffoldRow = appendActionRow(group.section,
                                          defaultKey,
                                          QString(),
                                          SystemdConflictStrategy::ReplaceValue,
                                          true,
                                          false,
                                          group.id);
        }

        if (scaffoldRow >= 0)
        {
            setActionRowMetadata(scaffoldRow, true, false, group.id);
        }
    }

    refreshStrategyForAllRows();
}

void SystemdWidget::refreshStrategyForAllRows()
{
    for (int row = 0; row < ui->actionsTableWidget->rowCount(); ++row)
    {
        attachStrategyComboBox(row);
        applyStrategyStateToRow(row);
    }
}

bool SystemdWidget::canRemoveActionRows(const QSet<int> &rowsToRemove, QString &errorText) const
{
    if (rowsToRemove.isEmpty() || !mandatoryRulesActive())
    {
        return true;
    }

    for (const int row : rowsToRemove)
    {
        if (isScaffoldRow(row))
        {
            errorText = QCoreApplication::translate("SystemdWidget",
                                                    "Scaffold rows cannot be removed. Use add/replace for value.");
            return false;
        }
    }

    const auto profile = SystemdManCatalog::mandatoryProfile(currentUnitType());
    for (const int row : rowsToRemove)
    {
        const QString section = sectionTextAtRow(row);
        const QString key = keyTextAtRow(row);
        const auto strictIt = std::find_if(
            profile.strictKeys.cbegin(), profile.strictKeys.cend(), [&section, &key](const auto &strict) {
                return equalsInsensitive(section, strict.first) && equalsInsensitive(key, strict.second);
            });
        if (strictIt != profile.strictKeys.cend())
        {
            errorText = QCoreApplication::translate("SystemdWidget",
                                                    "Section [%1], key '%2' is mandatory and cannot be removed.")
                            .arg(strictIt->first, strictIt->second);
            return false;
        }
    }

    QHash<QString, int> totalInGroup;
    QHash<QString, int> removingInGroup;
    for (const auto &group : profile.oneOfGroups)
    {
        totalInGroup[group.id] = 0;
        removingInGroup[group.id] = 0;
    }

    for (int row = 0; row < ui->actionsTableWidget->rowCount(); ++row)
    {
        const QString section = sectionTextAtRow(row);
        const QString key = keyTextAtRow(row);
        for (const auto &group : profile.oneOfGroups)
        {
            if (!equalsInsensitive(section, group.section))
            {
                continue;
            }

            const auto keyIt = std::find_if(group.keys.cbegin(), group.keys.cend(), [&key](const auto &groupKey) {
                return equalsInsensitive(key, groupKey);
            });
            if (keyIt == group.keys.cend())
            {
                continue;
            }

            totalInGroup[group.id] += 1;
            if (rowsToRemove.contains(row))
            {
                removingInGroup[group.id] += 1;
            }
            break;
        }
    }

    for (const auto &group : profile.oneOfGroups)
    {
        if (removingInGroup.value(group.id) > 0
            && (totalInGroup.value(group.id) - removingInGroup.value(group.id)) <= 0)
        {
            errorText = QCoreApplication::translate(
                "SystemdWidget",
                "Section [%1]: at least one key from mandatory group must remain.").arg(group.section);
            return false;
        }
    }

    return true;
}

bool SystemdWidget::isTruthySystemdValue(const QString &value) const
{
    const auto normalized = value.trimmed().toLower();
    return normalized == QStringLiteral("1")
        || normalized == QStringLiteral("yes")
        || normalized == QStringLiteral("true")
        || normalized == QStringLiteral("on");
}

void SystemdWidget::applyActionTableDelegates()
{
    auto *table = ui->actionsTableWidget;
    table->setItemDelegateForColumn(
        1,
        new ActionAutocompleteDelegate(
            [this](int) { return SystemdManCatalog::sectionsForUnitType(currentUnitType()); },
            ActionAutocompleteDelegate::Normalizer(),
            table));
    table->setItemDelegateForColumn(
        2,
        new ActionAutocompleteDelegate(
            [this](int row) { return keySuggestionsForRow(row); },
            [this](int row, const QString &rawKey) -> std::optional<QString> {
                QString normalizedKey;
                if (!normalizeKeyForRow(row, rawKey, normalizedKey))
                {
                    return std::nullopt;
                }
                return normalizedKey;
            },
            table));
}

void SystemdWidget::attachStrategyComboBox(int row)
{
    auto *table = ui->actionsTableWidget;
    auto *existingComboBox = qobject_cast<QComboBox *>(table->cellWidget(row, 0));
    int preferredStrategy = static_cast<int>(SystemdConflictStrategy::ReplaceValue);
    if (existingComboBox)
    {
        preferredStrategy = existingComboBox->currentIndex();
        table->removeCellWidget(row, 0);
        existingComboBox->deleteLater();
    }

    const bool allowReset = !isScaffoldRow(row);
    auto *strategyComboBox = createStrategyComboBox(ui->actionsTableWidget, allowReset);
    if (!allowReset && preferredStrategy == static_cast<int>(SystemdConflictStrategy::ResetKey))
    {
        preferredStrategy = static_cast<int>(SystemdConflictStrategy::ReplaceValue);
    }
    preferredStrategy = std::clamp(preferredStrategy, 0, strategyComboBox->count() - 1);
    strategyComboBox->setCurrentIndex(preferredStrategy);
    ui->actionsTableWidget->setCellWidget(row, 0, strategyComboBox);
    connect(strategyComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            [this, strategyComboBox](int) {
                for (int i = 0; i < ui->actionsTableWidget->rowCount(); ++i)
                {
                    if (ui->actionsTableWidget->cellWidget(i, 0) == strategyComboBox)
                    {
                        applyStrategyStateToRow(i);
                        break;
                    }
                }
            });
}

void SystemdWidget::attachDependencyTypeComboBox(int row, int typeIndex)
{
    if (row < 0 || row >= ui->dependenciesTableWidget->rowCount())
    {
        return;
    }

    auto *existingComboBox = qobject_cast<QComboBox *>(ui->dependenciesTableWidget->cellWidget(row, 0));
    if (existingComboBox)
    {
        ui->dependenciesTableWidget->removeCellWidget(row, 0);
        existingComboBox->deleteLater();
    }

    auto *comboBox = new QComboBox(ui->dependenciesTableWidget);
    comboBox->addItem(QCoreApplication::translate("SystemdWidget", "Changed"));
    comboBox->addItem(QCoreApplication::translate("SystemdWidget", "Presence Changed"));
    comboBox->setCurrentIndex(std::clamp(typeIndex, 0, comboBox->count() - 1));
    ui->dependenciesTableWidget->setCellWidget(row, 0, comboBox);
}

void SystemdWidget::applyStrategyStateToRow(int row) const
{
    if (row < 0 || row >= ui->actionsTableWidget->rowCount())
    {
        return;
    }

    auto *strategyComboBox = qobject_cast<QComboBox *>(ui->actionsTableWidget->cellWidget(row, 0));
    if (!strategyComboBox)
    {
        return;
    }

    auto *valueItem = ui->actionsTableWidget->item(row, 3);
    if (!valueItem)
    {
        valueItem = new QTableWidgetItem();
        ui->actionsTableWidget->setItem(row, 3, valueItem);
    }

    const bool resetSelected = strategyComboBox->currentIndex() == static_cast<int>(SystemdConflictStrategy::ResetKey);
    if (resetSelected)
    {
        valueItem->setText(QString());
        valueItem->setFlags(valueItem->flags() & ~Qt::ItemIsEditable);
    }
    else
    {
        valueItem->setFlags(valueItem->flags() | Qt::ItemIsEditable);
    }
}

QString SystemdWidget::buildUnitFileTextFromTable(bool with_header) const
{
    struct EditLine
    {
        std::string section;
        std::string key;
        std::string value;
        SystemdConflictStrategy strategy;
        int sourceRow;
    };

    std::ostringstream text;
    if (with_header)
    {
        text << GENERATED_EDITS_HEADER << "\n";
    }
    const auto editMode = static_cast<SystemdEditMode>(ui->unitEditModeComboBox->currentIndex());

    std::vector<EditLine> lines;
    lines.reserve(static_cast<size_t>(ui->actionsTableWidget->rowCount()));

    for (int row = 0; row < ui->actionsTableWidget->rowCount(); ++row)
    {
        auto *strategyComboBox = qobject_cast<QComboBox *>(ui->actionsTableWidget->cellWidget(row, 0));
        const QString section = sectionTextAtRow(row);
        const QString key = keyTextAtRow(row);
        const QString value = valueTextAtRow(row);
        if (section.isEmpty() || key.isEmpty())
        {
            continue;
        }

        const auto strategy = editMode == SystemdEditMode::Create
                            ? SystemdConflictStrategy::AddValue
                            : static_cast<SystemdConflictStrategy>(
                                  strategyComboBox ? strategyComboBox->currentIndex()
                                                   : static_cast<int>(SystemdConflictStrategy::ReplaceValue));
        if (isScaffoldRow(row) && !isStrictMandatoryRow(row) && !scaffoldGroupId(row).isEmpty()
            && strategy != SystemdConflictStrategy::ResetKey && value.trimmed().isEmpty())
        {
            continue;
        }

        lines.push_back(EditLine{section.toStdString(), key.toStdString(), value.toStdString(), strategy, row});
    }

    std::sort(lines.begin(), lines.end(), [](const EditLine &lhs, const EditLine &rhs) {
        if (lhs.section != rhs.section)
        {
            return lhs.section < rhs.section;
        }
        if (lhs.key != rhs.key)
        {
            return lhs.key < rhs.key;
        }
        return lhs.sourceRow < rhs.sourceRow;
    });

    std::string currentSection;
    bool firstSection = true;
    for (const auto &line : lines)
    {
        if (line.section != currentSection)
        {
            currentSection = line.section;
            if (!firstSection)
            {
                text << "\n";
            }
            text << "[" << currentSection << "]\n";
            firstSection = false;
        }

        switch (line.strategy)
        {
        case SystemdConflictStrategy::ReplaceValue:
            text << line.key << "=\n";
            if (!line.value.empty())
            {
                text << line.key << "=" << line.value << "\n";
            }
            break;
        case SystemdConflictStrategy::ResetKey:
            text << line.key << "=\n";
            break;
        case SystemdConflictStrategy::AddValue:
        default:
            text << line.key << "=" << line.value << "\n";
            break;
        }
    }

    return QString::fromStdString(text.str());
}

void SystemdWidget::fillTableFromUnitFileText(const QString &unitFileText)
{
    ui->actionsTableWidget->setRowCount(0);

    struct ParsedEdit
    {
        ParsedEdit()
            : section()
            , key()
            , value()
            , strategy(SystemdConflictStrategy::AddValue)
            , removed(false)
        {
        }

        QString section;
        QString key;
        QString value;
        SystemdConflictStrategy strategy{SystemdConflictStrategy::AddValue};
        bool removed{false};
    };

    struct PairState
    {
        PairState()
            : pendingReset(false)
            , replaceIndex(-1)
            , addIndexes()
        {
        }

        bool pendingReset{false};
        int replaceIndex{-1};
        QList<int> addIndexes;
    };

    QList<ParsedEdit> edits;
    QHash<QString, PairState> states;
    QString currentSection;
    const auto unitLines = unitFileText.split('\n');
    for (auto line : unitLines)
    {
        line = line.trimmed();
        if (line.isEmpty() || line.startsWith('#') || line.startsWith(';'))
        {
            continue;
        }

        if (line.startsWith('[') && line.endsWith(']'))
        {
            currentSection = line.mid(1, line.size() - 2).trimmed();
            continue;
        }

        const int equalsPos = line.indexOf('=');
        if (equalsPos < 0 || currentSection.isEmpty())
        {
            continue;
        }

        const QString key = line.left(equalsPos).trimmed();
        if (key.isEmpty())
        {
            continue;
        }

        const QString pairKey = makeEditKey(currentSection, key);
        auto &state = states[pairKey];
        const QString value = stripInlineComment(line.mid(equalsPos + 1));

        if (value.isEmpty())
        {
            state.pendingReset = true;
            continue;
        }

        if (state.pendingReset)
        {
            if (state.replaceIndex >= 0 && state.replaceIndex < edits.size())
            {
                edits[state.replaceIndex].removed = true;
            }
            for (const int index : state.addIndexes)
            {
                if (index >= 0 && index < edits.size())
                {
                    edits[index].removed = true;
                }
            }
            state.addIndexes.clear();

            ParsedEdit replaceEdit;
            replaceEdit.section = currentSection;
            replaceEdit.key = key;
            replaceEdit.value = value;
            replaceEdit.strategy = SystemdConflictStrategy::ReplaceValue;
            edits.push_back(replaceEdit);
            state.replaceIndex = edits.size() - 1;
            state.pendingReset = false;
            continue;
        }

        ParsedEdit addEdit;
        addEdit.section = currentSection;
        addEdit.key = key;
        addEdit.value = value;
        addEdit.strategy = SystemdConflictStrategy::AddValue;
        edits.push_back(addEdit);
        state.addIndexes.push_back(edits.size() - 1);
    }

    for (auto it = states.begin(); it != states.end(); ++it)
    {
        auto &state = it.value();
        if (!state.pendingReset)
        {
            continue;
        }

        if (state.replaceIndex >= 0 && state.replaceIndex < edits.size())
        {
            edits[state.replaceIndex].removed = true;
        }
        for (const int index : state.addIndexes)
        {
            if (index >= 0 && index < edits.size())
            {
                edits[index].removed = true;
            }
        }
        state.addIndexes.clear();

        const QStringList pair = it.key().split(QLatin1Char('\n'));
        if (pair.size() != 2)
        {
            continue;
        }

        ParsedEdit resetEdit;
        resetEdit.section = pair.at(0);
        resetEdit.key = pair.at(1);
        resetEdit.strategy = SystemdConflictStrategy::ResetKey;
        edits.push_back(resetEdit);
    }

    for (const auto &edit : edits)
    {
        if (edit.removed || edit.section.isEmpty() || edit.key.isEmpty())
        {
            continue;
        }

        appendActionRow(edit.section, edit.key, edit.value, edit.strategy, false, false, QString());
    }
}

void SystemdWidget::moveActionRows(bool moveUp)
{
    auto *table = ui->actionsTableWidget;
    if (!table || table->rowCount() < 2)
    {
        return;
    }

    const QList<int> rows = selectedRows(table);
    if (rows.isEmpty())
    {
        return;
    }

    struct ActionRowState
    {
        QString section;
        QString key;
        QString value;
        SystemdConflictStrategy strategy{SystemdConflictStrategy::ReplaceValue};
        bool scaffold{false};
        bool strictMandatory{false};
        QString groupId;
    };

    QList<ActionRowState> states;
    states.reserve(table->rowCount());
    for (int row = 0; row < table->rowCount(); ++row)
    {
        auto *strategyComboBox = qobject_cast<QComboBox *>(table->cellWidget(row, 0));
        const auto strategy = strategyComboBox
                            ? static_cast<SystemdConflictStrategy>(strategyComboBox->currentIndex())
                            : SystemdConflictStrategy::ReplaceValue;
        states.push_back(ActionRowState{
            sectionTextAtRow(row),
            keyTextAtRow(row),
            valueTextAtRow(row),
            strategy,
            isScaffoldRow(row),
            isStrictMandatoryRow(row),
            scaffoldGroupId(row),
        });
    }

    closePersistentEditors(table);
    table->clearFocus();
    table->setCurrentIndex(QModelIndex());
    const QSet<int> selectedSet = moveRowIndexes(rows, states.size(), moveUp, [&states](int lhs, int rhs) {
        std::swap(states[lhs], states[rhs]);
    });

    table->setUpdatesEnabled(false);
    table->setRowCount(0);
    for (const auto &state : states)
    {
        appendActionRow(state.section,
                        state.key,
                        state.value,
                        state.strategy,
                        state.scaffold,
                        state.strictMandatory,
                        state.groupId);
    }
    table->setUpdatesEnabled(true);

    QList<int> movedRows = selectedSet.values();
    std::sort(movedRows.begin(), movedRows.end());
    selectRows(table, movedRows);
    for (int row = 0; row < table->rowCount(); ++row)
    {
        applyStrategyStateToRow(row);
    }
}

void SystemdWidget::moveDependencyRows(bool moveUp)
{
    auto *table = ui->dependenciesTableWidget;
    if (!table || table->rowCount() < 2)
    {
        return;
    }

    const QList<int> rows = selectedRows(table);
    if (rows.isEmpty())
    {
        return;
    }

    struct DependencyRowState
    {
        int type{0};
        QString path;
    };

    QList<DependencyRowState> states;
    states.reserve(table->rowCount());
    for (int row = 0; row < table->rowCount(); ++row)
    {
        auto *combo = qobject_cast<QComboBox *>(table->cellWidget(row, 0));
        states.push_back(DependencyRowState{
            combo ? combo->currentIndex() : 0,
            table->item(row, 1) ? table->item(row, 1)->text() : QString(),
        });
    }

    closePersistentEditors(table);
    table->clearFocus();
    table->setCurrentIndex(QModelIndex());
    const QSet<int> selectedSet = moveRowIndexes(rows, states.size(), moveUp, [&states](int lhs, int rhs) {
        std::swap(states[lhs], states[rhs]);
    });

    table->setUpdatesEnabled(false);
    table->setRowCount(0);
    for (int row = 0; row < states.size(); ++row)
    {
        table->insertRow(row);
        attachDependencyTypeComboBox(row, states.at(row).type);
        table->setItem(row, 1, new QTableWidgetItem(states.at(row).path));
    }
    table->setUpdatesEnabled(true);

    QList<int> movedRows = selectedSet.values();
    std::sort(movedRows.begin(), movedRows.end());
    selectRows(table, movedRows);
}

void SystemdWidget::on_switchEditorModePushButton_clicked()
{
    if (m_textEditorMode)
    {
        fillTableFromUnitFileText(ui->unitFileTextEdit->toPlainText());
        m_textEditorMode = false;
        ensureScaffoldRows();
    }
    else
    {
        ui->unitFileTextEdit->setPlainText(buildUnitFileTextFromTable(false));
        m_textEditorMode = true;
    }

    updateEditModeAvailability();
    updateEditorModeUi();
}

void SystemdWidget::on_actionMoveUpButton_clicked()
{
    moveActionRows(true);
}

void SystemdWidget::on_actionMoveDownButton_clicked()
{
    moveActionRows(false);
}

void SystemdWidget::on_dependMoveUpButton_clicked()
{
    moveDependencyRows(true);
}

void SystemdWidget::on_dependMoveDownButton_clicked()
{
    moveDependencyRows(false);
}

void SystemdWidget::updateDependencyControls()
{
    if (!m_item)
    {
        return;
    }

    const bool enabled = m_item->property<bool>(SystemdItem::HAS_DEPENDENCIES);

    ui->dependenciesTableWidget->setEnabled(enabled);
    ui->addDependPushButton->setEnabled(enabled);
    ui->removeDependPushButton->setEnabled(enabled);
    ui->clearDependsPushButton->setEnabled(enabled);
    ui->dependMoveUpButton->setEnabled(enabled);
    ui->dependMoveDownButton->setEnabled(enabled);
}

bool SystemdWidget::validate()
{
    if (ui->unitNameLineEdit->text().trimmed().isEmpty())
    {
        QMessageBox::warning(this,
                             QCoreApplication::translate("SystemdWidget", "Validation error"),
                             QCoreApplication::translate("SystemdWidget", "Unit name is required."));
        ui->unitNameLineEdit->setFocus();
        return false;
    }

    if (ui->editUnitFileCheckBox->isChecked() && currentEditMode() != SystemdEditMode::Create)
    {
        const QString dropInBaseName = ui->dropInBaseNameLineEdit->text().trimmed();
        const QRegularExpression validPattern(QString::fromLatin1(DROP_IN_NAME_PATTERN));
        if (dropInBaseName.isEmpty() || !validPattern.match(dropInBaseName).hasMatch())
        {
            QMessageBox::warning(this,
                                 QCoreApplication::translate("SystemdWidget", "Validation error"),
                                 QCoreApplication::translate("SystemdWidget",
                                                             "Drop-in name must contain only letters, digits, '.', '_', '@' or '-'."));
            ui->dropInBaseNameLineEdit->setFocus();
            return false;
        }
    }

    if (ui->editUnitFileCheckBox->isChecked() && !m_textEditorMode)
    {
        QString tableError;
        if (!validateTableMode(tableError))
        {
            QMessageBox::warning(this,
                                 QCoreApplication::translate("SystemdWidget", "Validation error"),
                                 tableError);
            ui->actionsTableWidget->setFocus();
            return false;
        }
    }

    return true;
}

bool SystemdWidget::validateTableMode(QString &errorText) const
{
    struct RowData
    {
        int row{0};
        QString section;
        QString key;
        QString value;
        SystemdConflictStrategy strategy{SystemdConflictStrategy::AddValue};
    };

    QList<RowData> allRows;
    QHash<QString, QList<RowData>> rowsByKey;
    const auto editMode = currentEditMode();
    const auto type = currentUnitType();
    const bool mandatoryActive = mandatoryRulesActive();
    const auto mandatoryProfile = SystemdManCatalog::mandatoryProfile(type);
    QHash<QString, bool> mandatoryGroupsSatisfied;
    for (const auto &group : mandatoryProfile.oneOfGroups)
    {
        mandatoryGroupsSatisfied[group.id] = false;
    }

    bool hasExecStartValue = false;
    bool hasExecStopValue = false;
    bool hasRemainAfterExitTrue = false;

    for (int row = 0; row < ui->actionsTableWidget->rowCount(); ++row)
    {
        const QString section = sectionTextAtRow(row);
        const QString key = keyTextAtRow(row);
        const QString value = valueTextAtRow(row);
        auto *strategyComboBox = qobject_cast<QComboBox *>(ui->actionsTableWidget->cellWidget(row, 0));
        const auto strategy = editMode == SystemdEditMode::Create
                            ? SystemdConflictStrategy::AddValue
                            : static_cast<SystemdConflictStrategy>(
                                  strategyComboBox ? strategyComboBox->currentIndex()
                                                   : static_cast<int>(SystemdConflictStrategy::ReplaceValue));

        if (section.isEmpty())
        {
            errorText = QCoreApplication::translate("SystemdWidget", "Row %1: Section is required.").arg(row + 1);
            return false;
        }

        if (key.isEmpty())
        {
            errorText = QCoreApplication::translate("SystemdWidget", "Row %1: Key is required.").arg(row + 1);
            return false;
        }

        bool strictMandatoryRow = false;
        const auto strictMatch = std::find_if(
            mandatoryProfile.strictKeys.cbegin(), mandatoryProfile.strictKeys.cend(), [&section, &key](const auto &strict) {
                return equalsInsensitive(section, strict.first) && equalsInsensitive(key, strict.second);
            });
        strictMandatoryRow = strictMatch != mandatoryProfile.strictKeys.cend();

        bool groupMandatoryRow = false;
        QString groupId = scaffoldGroupId(row);
        if (mandatoryActive && !groupId.isEmpty())
        {
            const auto groupIt = std::find_if(mandatoryProfile.oneOfGroups.cbegin(),
                                              mandatoryProfile.oneOfGroups.cend(),
                                              [&groupId](const auto &group) {
                                                  return group.id == groupId;
                                              });
            if (groupIt == mandatoryProfile.oneOfGroups.cend())
            {
                errorText = QCoreApplication::translate("SystemdWidget",
                                                        "Row %1: Mandatory key group metadata is invalid.")
                                .arg(row + 1);
                return false;
            }

            if (!equalsInsensitive(section, groupIt->section))
            {
                errorText = QCoreApplication::translate("SystemdWidget",
                                                        "Row %1: Section must be [%2] for mandatory key group.")
                                .arg(row + 1)
                                .arg(groupIt->section);
                return false;
            }

            const auto keyMatch = std::find_if(groupIt->keys.cbegin(), groupIt->keys.cend(), [&key](const auto &groupKey) {
                return equalsInsensitive(key, groupKey);
            });
            if (keyMatch == groupIt->keys.cend())
            {
                errorText = QCoreApplication::translate(
                    "SystemdWidget",
                    "Row %1: Key '%2' is not allowed for mandatory key group. Allowed keys: %3.")
                                .arg(row + 1)
                                .arg(key)
                                .arg(groupIt->keys.join(QStringLiteral(", ")));
                return false;
            }
            groupMandatoryRow = true;
        }
        else
        {
            for (const auto &group : mandatoryProfile.oneOfGroups)
            {
                if (!equalsInsensitive(section, group.section))
                {
                    continue;
                }

                const auto keyMatch = std::find_if(group.keys.cbegin(), group.keys.cend(), [&key](const auto &groupKey) {
                    return equalsInsensitive(key, groupKey);
                });
                if (keyMatch != group.keys.cend())
                {
                    groupMandatoryRow = true;
                    groupId = group.id;
                    break;
                }
            }
        }

        const bool valueRequired = strategy != SystemdConflictStrategy::ResetKey
                                && (!mandatoryActive || strictMandatoryRow || !groupMandatoryRow);
        if (valueRequired && value.trimmed().isEmpty())
        {
            errorText = QCoreApplication::translate("SystemdWidget",
                                                    "Row %1: Value is required for add/replace strategy.")
                            .arg(row + 1);
            return false;
        }

        const auto rowData = RowData{row, section, key, value, strategy};
        allRows.push_back(rowData);
        rowsByKey[makeEditKey(section, key)].push_back(rowData);

        if (mandatoryActive && groupMandatoryRow && strategy != SystemdConflictStrategy::ResetKey && !value.trimmed().isEmpty())
        {
            mandatoryGroupsSatisfied[groupId] = true;
        }

        if (mandatoryActive && type == SystemdUnitType::Service && equalsInsensitive(section, "Service")
            && strategy != SystemdConflictStrategy::ResetKey && !value.trimmed().isEmpty())
        {
            if (equalsInsensitive(key, "ExecStart"))
            {
                hasExecStartValue = true;
            }
            if (equalsInsensitive(key, "ExecStop"))
            {
                hasExecStopValue = true;
            }
            if (equalsInsensitive(key, "RemainAfterExit") && isTruthySystemdValue(value))
            {
                hasRemainAfterExitTrue = true;
            }
        }
    }

    for (auto it = rowsByKey.begin(); it != rowsByKey.end(); ++it)
    {
        const auto &rows = it.value();
        int replaceCount = 0;
        int resetCount = 0;
        int addCount = 0;
        int replaceRow = -1;
        QString section;
        QString key;

        for (const auto &rowData : rows)
        {
            section = rowData.section;
            key = rowData.key;
            switch (rowData.strategy)
            {
            case SystemdConflictStrategy::ReplaceValue:
                ++replaceCount;
                replaceRow = rowData.row;
                break;
            case SystemdConflictStrategy::ResetKey:
                ++resetCount;
                break;
            case SystemdConflictStrategy::AddValue:
            default:
                ++addCount;
                break;
            }
        }

        if (replaceCount > 1)
        {
            errorText = QCoreApplication::translate(
                "SystemdWidget",
                "Section [%1], key '%2': replace strategy can be used only once.").arg(section, key);
            return false;
        }

        if (resetCount > 1)
        {
            errorText = QCoreApplication::translate(
                "SystemdWidget",
                "Section [%1], key '%2': reset strategy can be used only once.").arg(section, key);
            return false;
        }

        if (resetCount > 0 && replaceCount > 0)
        {
            errorText = QCoreApplication::translate(
                "SystemdWidget",
                "Section [%1], key '%2': reset and replace cannot be combined.").arg(section, key);
            return false;
        }

        if (resetCount > 0 && addCount > 0)
        {
            errorText = QCoreApplication::translate(
                "SystemdWidget",
                "Section [%1], key '%2': reset and add cannot be combined.").arg(section, key);
            return false;
        }

        if (replaceCount == 1)
        {
            for (const auto &rowData : rows)
            {
                if (rowData.strategy == SystemdConflictStrategy::AddValue && rowData.row < replaceRow)
                {
                    errorText = QCoreApplication::translate(
                        "SystemdWidget",
                        "Section [%1], key '%2': add rows cannot be placed before replace.").arg(section, key);
                    return false;
                }
            }
        }
    }

    if (!mandatoryActive)
    {
        return true;
    }

    for (const auto &strict : mandatoryProfile.strictKeys)
    {
        const bool strictSatisfied = std::any_of(allRows.cbegin(),
                                                 allRows.cend(),
                                                 [&strict](const RowData &row) {
                                                     return equalsInsensitive(row.section, strict.first)
                                                         && equalsInsensitive(row.key, strict.second)
                                                         && row.strategy != SystemdConflictStrategy::ResetKey
                                                         && !row.value.trimmed().isEmpty();
                                                 });
        if (!strictSatisfied)
        {
            errorText = QCoreApplication::translate("SystemdWidget",
                                                    "Section [%1], key '%2': value is required.")
                            .arg(strict.first, strict.second);
            return false;
        }
    }

    for (const auto &group : mandatoryProfile.oneOfGroups)
    {
        if (mandatoryGroupsSatisfied.value(group.id))
        {
            continue;
        }

        errorText = QCoreApplication::translate("SystemdWidget",
                                                "Section [%1]: at least one of keys is required: %2.")
                        .arg(group.section, group.keys.join(QStringLiteral(", ")));
        return false;
    }

    if (mandatoryProfile.requireServiceRemainAfterExitWithExecStopOnly
        && hasExecStopValue && !hasExecStartValue && !hasRemainAfterExitTrue)
    {
        errorText = QCoreApplication::translate(
            "SystemdWidget",
            "Section [Service]: RemainAfterExit must be true when using ExecStop without ExecStart.");
        return false;
    }

    return true;
}

QString SystemdWidget::name() const
{
    return QObject::tr("Systemd");
}

} // namespace preferences
