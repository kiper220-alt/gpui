#include "systemdwidget.h"
#include "ui_systemdwidget.h"

#include "common/commonutils.h"
#include "systemditem.h"

#include <mvvm/factories/viewmodelfactory.h>
#include <mvvm/viewmodel/viewmodeldelegate.h>

#include <QDataWidgetMapper>

namespace preferences 
{

SystemdWidget::SystemdWidget(QWidget* parent, SystemdItem* item)
    : BasePreferenceWidget(parent)
    , m_item(item)
    , view_model(nullptr)
    , delegate(std::make_unique<ModelView::ViewModelDelegate>())
    , ui(new Ui::SystemdWidget())
{
    ui->setupUi(this);
    connect(this->ui->addActionPushButton, SIGNAL(clicked()), this,  SLOT(on_actionAddButton_clicked()));
    connect(this->ui->clearActionsPushButton, SIGNAL(clicked()), this,  SLOT(on_actionsClearButton_clicked()));
    connect(this->ui->removeActionPushButton, SIGNAL(clicked()), this,  SLOT(on_actionRemoveButton_clicked()));
    connect(this->ui->addDependPushButton, SIGNAL(clicked()), this,  SLOT(on_dependAddButton_clicked()));
    connect(this->ui->clearDependsPushButton, SIGNAL(clicked()), this,  SLOT(on_dependsClearButton_clicked()));
    connect(this->ui->removeDependPushButton, SIGNAL(clicked()), this,  SLOT(on_dependRemoveButton_clicked()));
}
SystemdWidget::~SystemdWidget() 
{
    delete ui;
}

void SystemdWidget::setItem(ModelView::SessionItem *item)
{
    view_model = ModelView::Factory::CreatePropertyFlatViewModel(item->model());
    view_model->setRootSessionItem(item);

    m_item = dynamic_cast<preferences::SystemdItem *>(item);

    mapper = std::make_unique<QDataWidgetMapper>();

    mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    mapper->setOrientation(Qt::Vertical);

    mapper->setModel(view_model.get());
    mapper->setItemDelegate(delegate.get());
    mapper->setRootIndex(QModelIndex());

    mapper->addMapping(ui->unitLineEdit, 0);
    // mapper->addMapping(ui->changeUnitStateComboBox, 1, "currentIndex"); - not valid mapping
    if (m_item->property<int>(preferences::SystemdItem::STATE) != 0) 
    {
        ui->changeUnitStateCheckBox->setChecked(true);
        ui->changeUnitStateComboBox->setCurrentIndex(m_item->property<int>(preferences::SystemdItem::STATE) - 1);
    }
    mapper->addMapping(ui->nowCheckBox, 2);
    mapper->addMapping(ui->editUnitFileCheckBox, 3);
    mapper->addMapping(ui->restartCheckBox, 4);

    this->readEditTable();
    this->readDependencyTable();

    mapper->setCurrentModelIndex(view_model->index(0, 1));
}

void SystemdWidget::readEditTable()
{
    const auto table = this->ui->actionsTableWidget;
    const auto items = m_item->editItems();
    if (items.size() > 0)
    {
        for (auto item : items)
        {
            this->on_actionAddButton_clicked();
            auto type = item->property<int>(preferences::SystemdEditItem::TYPE);
            auto section = item->property<std::string>(preferences::SystemdEditItem::SECTION);
            auto key = item->property<std::string>(preferences::SystemdEditItem::KEY);
            auto value = item->property<std::string>(preferences::SystemdEditItem::VALUE);

            dynamic_cast<QComboBox*>(table->cellWidget(table->rowCount() - 1, 0))->setCurrentIndex(type);
            table->setItem(table->rowCount() - 1, 1, new QTableWidgetItem(section.c_str()));
            table->setItem(table->rowCount() - 1, 2, new QTableWidgetItem(key.c_str()));
            table->setItem(table->rowCount() - 1, 3, new QTableWidgetItem(value.c_str()));
        }
    }
}
void SystemdWidget::writeEditTable()
{
    const auto table = ui->actionsTableWidget;
    m_item->editLength(table->rowCount());

    const auto items = m_item->editItems();

    for (int i = 0; i < items.size(); ++i)
    {
        const auto edit = items.at(i);
        edit->setProperty(
            preferences::SystemdEditItem::TYPE,
            dynamic_cast<QComboBox*>(table->cellWidget(i, 0))->currentIndex()
        );

        auto widget = table->item(i, 1);
        auto text = widget ? widget->text() : QString();

        edit->setProperty(
            preferences::SystemdEditItem::SECTION,
            text.toStdString()
        );

        widget = table->item(i, 2);
        text = widget ? widget->text() : QString();

        edit->setProperty(
            preferences::SystemdEditItem::KEY,
            text.toStdString()
        );
        
        widget = table->item(i, 3);
        text = widget ? widget->text() : QString();

        edit->setProperty(
            preferences::SystemdEditItem::VALUE, 
            text.toStdString()
        );
    }
    if (items.size() == 0)
    {
        m_item->setProperty(
            preferences::SystemdItem::EDIT,
            false
        );
    }
}

void SystemdWidget::readDependencyTable()
{
    const auto table = this->ui->dependenciesTableWidget;
    const auto items = m_item->depItems();
    if (items.size() > 0)
    {
        for (auto item : items)
        {
            this->on_dependAddButton_clicked();
            auto type = item->property<int>(preferences::SystemdDependencyItem::TYPE);
            auto path = item->property<std::string>(preferences::SystemdDependencyItem::PATH);

            dynamic_cast<QComboBox*>(table->cellWidget(table->rowCount() - 1, 0))->setCurrentIndex(type);
            table->setItem(table->rowCount() - 1, 1, new QTableWidgetItem(path.c_str()));
        }
    }
}
void SystemdWidget::writeDependencyTable()
{
    const auto table = ui->dependenciesTableWidget;
    m_item->depLength(table->rowCount());

    const auto items = m_item->depItems();

    for (int i = 0; i < items.size(); ++i)
    {
        const auto dependence = items.at(i);
        dependence->setProperty(
            preferences::SystemdDependencyItem::TYPE,
            dynamic_cast<QComboBox*>(table->cellWidget(i, 0))->currentIndex()
        );

        auto widget = table->item(i, 1);
        auto text = widget ? widget->text() : QString();

        dependence->setProperty(
            preferences::SystemdDependencyItem::PATH,
            text.toStdString()
        );
    }
}

bool SystemdWidget::validate()
{
    return true;
}

QString SystemdWidget::name() const {
    return "Systemd";
}

}