#include "systemdwidget.h"

#include "ui_systemdwidget.h"

#include <algorithm>
#include <sstream>
#include <vector>
#include <QRegularExpression>

#include "common/commonitem.h"
#include "systemditem.h"
#include "systemdunitsyntaxhighlighter.h"

namespace
{

const char *GENERATED_EDITS_HEADER = "# GPUI-GENERATED: edits-from-table";

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

QComboBox *createStrategyComboBox(QWidget *parent)
{
    auto *comboBox = new QComboBox(parent);
    comboBox->addItem(QCoreApplication::translate("SystemdWidget", "Replace value"));
    comboBox->addItem(QCoreApplication::translate("SystemdWidget", "Add value"));
    comboBox->addItem(QCoreApplication::translate("SystemdWidget", "Reset key"));
    return comboBox;
}

QString makeEditKey(const QString &section, const QString &key)
{
    return section + QLatin1Char('\n') + key;
}

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

void swapRows(QTableWidget *table, int firstRow, int secondRow)
{
    if (!table || firstRow == secondRow || firstRow < 0 || secondRow < 0
        || firstRow >= table->rowCount() || secondRow >= table->rowCount())
    {
        return;
    }

    for (int col = 0; col < table->columnCount(); ++col)
    {
        auto *firstItem = table->takeItem(firstRow, col);
        auto *secondItem = table->takeItem(secondRow, col);
        table->setItem(firstRow, col, secondItem);
        table->setItem(secondRow, col, firstItem);

        QWidget *firstWidget = table->cellWidget(firstRow, col);
        QWidget *secondWidget = table->cellWidget(secondRow, col);
        if (firstWidget)
        {
            table->removeCellWidget(firstRow, col);
        }
        if (secondWidget)
        {
            table->removeCellWidget(secondRow, col);
        }
        if (firstWidget)
        {
            table->setCellWidget(secondRow, col, firstWidget);
        }
        if (secondWidget)
        {
            table->setCellWidget(firstRow, col, secondWidget);
        }
    }
}

void selectRows(QTableWidget *table, const QList<int> &rows)
{
    if (!table || !table->selectionModel())
    {
        return;
    }

    table->clearSelection();
    bool firstSelected = false;
    for (const int row : rows)
    {
        if (row >= 0 && row < table->rowCount())
        {
            const auto index = table->model()->index(row, 0);
            table->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
            if (!firstSelected)
            {
                table->setCurrentIndex(index);
                firstSelected = true;
            }
        }
    }
}

void moveSelectedRows(QTableWidget *table, bool moveUp)
{
    if (!table || table->rowCount() < 2)
    {
        return;
    }

    QList<int> rows = selectedRows(table);
    if (rows.isEmpty())
    {
        return;
    }

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

            swapRows(table, row, row - 1);
            selectedSet.remove(row);
            selectedSet.insert(row - 1);
        }
    }
    else
    {
        for (int i = rows.size() - 1; i >= 0; --i)
        {
            const int row = rows.at(i);
            if (row == table->rowCount() - 1 || selectedSet.contains(row + 1))
            {
                continue;
            }

            swapRows(table, row, row + 1);
            selectedSet.remove(row);
            selectedSet.insert(row + 1);
        }
    }

    rows = selectedSet.values();
    std::sort(rows.begin(), rows.end());
    selectRows(table, rows);
}

} // namespace

namespace preferences
{

SystemdWidget::SystemdWidget(QWidget *parent, SystemdItem *item)
    : BasePreferenceWidget(parent)
    , m_item(item)
    , ui(new Ui::SystemdWidget())
{
    ui->setupUi(this);
    ui->actionsTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->actionsTableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->actionsTableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->actionsTableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    ui->dependenciesTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->dependenciesTableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    (void)new SystemdUnitSyntaxHighlighter(ui->unitFileTextEdit->document());

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
            [this](int) { updateEditModeAvailability(); });
    connect(ui->unitEditModeComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            [this](int) { updateEditModeAvailability(); });
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
    ui->dropInNameLineEdit->setText(QString::fromStdString(m_item->property<std::string>(SystemdItem::DROP_IN_NAME)));
    ui->unitFileTextEdit->setPlainText(QString::fromStdString(m_item->property<std::string>(SystemdItem::UNIT_FILE_TEXT)));
    m_textEditorMode = m_item->property<int>(SystemdItem::UNIT_FILE_MODE) == static_cast<int>(SystemdUnitFileMode::Text);

    readEditTable();

    readDependencyTable();

    updateUiForUnitType();
    updatePolicyTargetControls();
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
        const int row = ui->actionsTableWidget->rowCount();
        ui->actionsTableWidget->insertRow(row);
        ui->actionsTableWidget->setItem(
            row,
            1,
            new QTableWidgetItem(QString::fromStdString(edit->property<std::string>(SystemdEditItem::SECTION))));
        ui->actionsTableWidget->setItem(
            row,
            2,
            new QTableWidgetItem(QString::fromStdString(edit->property<std::string>(SystemdEditItem::KEY))));
        ui->actionsTableWidget->setItem(
            row,
            3,
            new QTableWidgetItem(QString::fromStdString(edit->property<std::string>(SystemdEditItem::VALUE))));
        attachStrategyComboBox(row);
        if (auto *strategyComboBox = qobject_cast<QComboBox *>(ui->actionsTableWidget->cellWidget(row, 0)))
        {
            strategyComboBox->setCurrentIndex(edit->property<int>(SystemdEditItem::STRATEGY));
        }
        applyStrategyStateToRow(row);
    }
}

void SystemdWidget::writeEditTable()
{
    if (!m_item)
    {
        return;
    }

    auto *table = ui->actionsTableWidget;
    m_item->editLength(table->rowCount());

    const auto items = m_item->editItems();
    const auto editMode = static_cast<SystemdEditMode>(ui->unitEditModeComboBox->currentIndex());
    for (int i = 0; i < static_cast<int>(items.size()); ++i)
    {
        const auto *sectionWidget = table->item(i, 1);
        const auto *keyWidget = table->item(i, 2);
        const auto *valueWidget = table->item(i, 3);
        auto *strategyComboBox = qobject_cast<QComboBox *>(table->cellWidget(i, 0));

        items.at(i)->setProperty(SystemdEditItem::SECTION,
                                 sectionWidget ? sectionWidget->text().toStdString() : std::string());
        items.at(i)->setProperty(SystemdEditItem::KEY,
                                 keyWidget ? keyWidget->text().toStdString() : std::string());
        items.at(i)->setProperty(SystemdEditItem::VALUE,
                                 valueWidget ? valueWidget->text().toStdString() : std::string());
        const int strategy = editMode == SystemdEditMode::Create
                           ? static_cast<int>(SystemdConflictStrategy::AddValue)
                           : (strategyComboBox ? strategyComboBox->currentIndex()
                                               : static_cast<int>(SystemdConflictStrategy::ReplaceValue));
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
        on_dependAddButton_clicked();
        const int row = ui->dependenciesTableWidget->rowCount() - 1;
        auto *combo = qobject_cast<QComboBox *>(ui->dependenciesTableWidget->cellWidget(row, 0));
        if (combo)
        {
            combo->setCurrentIndex(dep->property<int>(SystemdDependencyItem::TYPE));
        }
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
}

void SystemdWidget::updateStateControls()
{
    const bool enabled = ui->changeUnitStateCheckBox->isChecked();
    ui->changeUnitStateComboBox->setEnabled(enabled);
    ui->nowCheckBox->setEnabled(enabled);
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
        ui->policyTargetHintLabel->clear();
        ui->policyTargetHintLabel->setVisible(false);
    }
}

void SystemdWidget::updateEditControls()
{
    const bool enabled = ui->editUnitFileCheckBox->isChecked();

    ui->dropInNameLineEdit->setEnabled(enabled);
    ui->actionsTableWidget->setEnabled(enabled);
    ui->addActionPushButton->setEnabled(enabled);
    ui->removeActionPushButton->setEnabled(enabled);
    ui->clearActionsPushButton->setEnabled(enabled);
    ui->actionMoveUpButton->setEnabled(enabled);
    ui->actionMoveDownButton->setEnabled(enabled);
    ui->unitFileTextEdit->setEnabled(enabled);
    ui->switchEditorModePushButton->setEnabled(enabled);
    ui->dropInLabel->setVisible(enabled);
    ui->dropInNameLineEdit->setVisible(enabled);
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
    ui->dropInNameLineEdit->setVisible(showDropIn);

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
    if (!allowedModes.contains(ui->unitEditModeComboBox->currentIndex()))
    {
        ui->unitEditModeComboBox->setCurrentIndex(allowedModes.first());
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

void SystemdWidget::attachStrategyComboBox(int row)
{
    auto *strategyComboBox = createStrategyComboBox(ui->actionsTableWidget);
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

void SystemdWidget::applyStrategyStateToRow(int row) const
{
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
        const auto *sectionItem = ui->actionsTableWidget->item(row, 1);
        const auto *keyItem = ui->actionsTableWidget->item(row, 2);
        const auto *valueItem = ui->actionsTableWidget->item(row, 3);
        auto *strategyComboBox = qobject_cast<QComboBox *>(ui->actionsTableWidget->cellWidget(row, 0));

        const std::string section = sectionItem ? sectionItem->text().trimmed().toStdString() : std::string();
        const std::string key = keyItem ? keyItem->text().trimmed().toStdString() : std::string();
        const std::string value = valueItem ? valueItem->text().toStdString() : std::string();
        if (section.empty() || key.empty())
        {
            continue;
        }

        const auto strategy = editMode == SystemdEditMode::Create
                            ? SystemdConflictStrategy::AddValue
                            : static_cast<SystemdConflictStrategy>(
                                  strategyComboBox ? strategyComboBox->currentIndex()
                                                   : static_cast<int>(SystemdConflictStrategy::ReplaceValue));
        lines.push_back(EditLine{section, key, value, strategy, row});
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
        QString section;
        QString key;
        QString value;
        SystemdConflictStrategy strategy{SystemdConflictStrategy::AddValue};
        bool removed{false};
    };

    struct PairState
    {
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

        on_actionAddButton_clicked();
        const int row = ui->actionsTableWidget->rowCount() - 1;
        ui->actionsTableWidget->setItem(row, 1, new QTableWidgetItem(edit.section));
        ui->actionsTableWidget->setItem(row, 2, new QTableWidgetItem(edit.key));

        if (auto *combo = qobject_cast<QComboBox *>(ui->actionsTableWidget->cellWidget(row, 0)))
        {
            combo->setCurrentIndex(static_cast<int>(edit.strategy));
        }
        ui->actionsTableWidget->setItem(row, 3, new QTableWidgetItem(edit.value));
        applyStrategyStateToRow(row);
    }
}

void SystemdWidget::on_switchEditorModePushButton_clicked()
{
    if (m_textEditorMode)
    {
        fillTableFromUnitFileText(ui->unitFileTextEdit->toPlainText());
        m_textEditorMode = false;
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
    moveSelectedRows(ui->actionsTableWidget, true);
    for (int row = 0; row < ui->actionsTableWidget->rowCount(); ++row)
    {
        applyStrategyStateToRow(row);
    }
}

void SystemdWidget::on_actionMoveDownButton_clicked()
{
    moveSelectedRows(ui->actionsTableWidget, false);
    for (int row = 0; row < ui->actionsTableWidget->rowCount(); ++row)
    {
        applyStrategyStateToRow(row);
    }
}

void SystemdWidget::on_dependMoveUpButton_clicked()
{
    moveSelectedRows(ui->dependenciesTableWidget, true);
}

void SystemdWidget::on_dependMoveDownButton_clicked()
{
    moveSelectedRows(ui->dependenciesTableWidget, false);
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

    QHash<QString, QList<RowData>> rowsByKey;
    const auto editMode = static_cast<SystemdEditMode>(ui->unitEditModeComboBox->currentIndex());
    for (int row = 0; row < ui->actionsTableWidget->rowCount(); ++row)
    {
        const auto *sectionItem = ui->actionsTableWidget->item(row, 1);
        const auto *keyItem = ui->actionsTableWidget->item(row, 2);
        const auto *valueItem = ui->actionsTableWidget->item(row, 3);
        auto *strategyComboBox = qobject_cast<QComboBox *>(ui->actionsTableWidget->cellWidget(row, 0));

        const QString section = sectionItem ? sectionItem->text().trimmed() : QString();
        const QString key = keyItem ? keyItem->text().trimmed() : QString();
        const QString value = valueItem ? valueItem->text() : QString();
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

        if (strategy != SystemdConflictStrategy::ResetKey && value.trimmed().isEmpty())
        {
            errorText = QCoreApplication::translate("SystemdWidget",
                                                    "Row %1: Value is required for add/replace strategy.")
                            .arg(row + 1);
            return false;
        }

        rowsByKey[makeEditKey(section, key)].push_back(RowData{row, section, key, value, strategy});
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

    return true;
}

QString SystemdWidget::name() const
{
    return QObject::tr("Systemd");
}

} // namespace preferences
