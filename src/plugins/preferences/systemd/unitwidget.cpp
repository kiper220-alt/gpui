#include "unitwidget.h"

#include "ui_unitwidget.h"

#include <algorithm>

#include "systemditem.h"

namespace
{

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

} // namespace

namespace preferences
{

SystemdWidget::SystemdWidget(QWidget *parent, SystemdItem *item)
    : BasePreferenceWidget(parent)
    , m_item(item)
    , ui(new Ui::SystemdWidget())
{
    ui->setupUi(this);
    ui->actionsTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->dependenciesTableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->dependenciesTableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

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
    ui->conflictStrategyComboBox->setCurrentIndex(m_item->property<int>(SystemdItem::CONFLICT_STRATEGY));

    readEditTable();

    readDependencyTable();

    updateUiForUnitType();
    updateStateControls();
    updateEditControls();
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
            0,
            new QTableWidgetItem(QString::fromStdString(edit->property<std::string>(SystemdEditItem::SECTION))));
        ui->actionsTableWidget->setItem(
            row,
            1,
            new QTableWidgetItem(QString::fromStdString(edit->property<std::string>(SystemdEditItem::KEY))));
        ui->actionsTableWidget->setItem(
            row,
            2,
            new QTableWidgetItem(QString::fromStdString(edit->property<std::string>(SystemdEditItem::VALUE))));
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
    for (int i = 0; i < static_cast<int>(items.size()); ++i)
    {
        const auto *sectionWidget = table->item(i, 0);
        const auto *keyWidget = table->item(i, 1);
        const auto *valueWidget = table->item(i, 2);

        items.at(i)->setProperty(SystemdEditItem::SECTION,
                                 sectionWidget ? sectionWidget->text().toStdString() : std::string());
        items.at(i)->setProperty(SystemdEditItem::KEY,
                                 keyWidget ? keyWidget->text().toStdString() : std::string());
        items.at(i)->setProperty(SystemdEditItem::VALUE,
                                 valueWidget ? valueWidget->text().toStdString() : std::string());
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

void SystemdWidget::updateEditControls()
{
    const bool enabled = ui->editUnitFileCheckBox->isChecked();

    ui->unitEditModeComboBox->setEnabled(enabled);
    ui->dropInNameLineEdit->setEnabled(enabled);
    ui->conflictStrategyComboBox->setEnabled(enabled);
    ui->actionsTableWidget->setEnabled(enabled);
    ui->addActionPushButton->setEnabled(enabled);
    ui->removeActionPushButton->setEnabled(enabled);
    ui->clearActionsPushButton->setEnabled(enabled);
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

    return true;
}

QString SystemdWidget::name() const
{
    return QObject::tr("Systemd");
}

} // namespace preferences
