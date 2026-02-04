#include "unitwidget.h"

#include <algorithm>

#include "ui_unitwidget.h"

#include "systemditem.h"

namespace
{

QWidget *createDependenceComboBox(QWidget *parent)
{
    auto *comboBox = new QComboBox(parent);
    comboBox->addItem(QObject::tr("Changed"));
    comboBox->addItem(QObject::tr("Presence Changed"));
    return comboBox;
}

} // namespace

namespace preferences
{

void SystemdWidget::submit()
{
    if (!m_item || !validate())
    {
        return;
    }

    m_item->setProperty(SystemdItem::UNIT, ui->unitNameLineEdit->text().toStdString());

    const bool changeState = ui->changeUnitStateCheckBox->isChecked();
    m_item->setProperty(SystemdItem::STATE,
                        changeState ? ui->changeUnitStateComboBox->currentIndex() + static_cast<int>(SystemdState::Enable)
                                    : static_cast<int>(SystemdState::AsIs));
    m_item->setProperty(SystemdItem::STATE_NOW, changeState ? ui->nowCheckBox->isChecked() : false);

    m_item->setProperty(SystemdItem::APPLY_MODE, ui->applyModeComboBox->currentIndex());
    m_item->setProperty(SystemdItem::POLICY_TARGET, ui->policyTargetComboBox->currentIndex());

    m_item->setProperty(SystemdItem::EDIT, ui->editUnitFileCheckBox->isChecked());
    m_item->setProperty(SystemdItem::EDIT_MODE, ui->unitEditModeComboBox->currentIndex());
    m_item->setProperty(SystemdItem::DROP_IN_NAME, ui->dropInNameLineEdit->text().toStdString());
    m_item->setProperty(SystemdItem::CONFLICT_STRATEGY, ui->conflictStrategyComboBox->currentIndex());

    writeEditTable();

    if (m_item->property<bool>(SystemdItem::HAS_DEPENDENCIES))
    {
        m_item->setProperty(SystemdItem::DEPENDENCY, ui->dependenciesTableWidget->rowCount() > 0);
        writeDependencyTable();
    }
    else
    {
        m_item->setProperty(SystemdItem::DEPENDENCY, false);
        m_item->depLength(0);
    }

    emit dataChanged();
}

void SystemdWidget::on_actionAddButton_clicked() const
{
    const int rows = ui->actionsTableWidget->rowCount();
    ui->actionsTableWidget->insertRow(rows);
}

void SystemdWidget::on_actionsClearButton_clicked()
{
    ui->actionsTableWidget->setRowCount(0);
}

void SystemdWidget::on_actionRemoveButton_clicked()
{
    auto *table = ui->actionsTableWidget;

    QSet<int> rows;
    for (const auto &index : table->selectionModel()->selectedRows())
    {
        rows.insert(index.row());
    }

    QList<int> rowsList = rows.values();
    std::sort(rowsList.begin(), rowsList.end(), std::greater<>());

    for (const int row : rowsList)
    {
        table->removeRow(row);
    }
}

void SystemdWidget::on_dependAddButton_clicked()
{
    const int rows = ui->dependenciesTableWidget->rowCount();
    ui->dependenciesTableWidget->insertRow(rows);
    ui->dependenciesTableWidget->setCellWidget(rows, 0, createDependenceComboBox(ui->dependenciesTableWidget));
}

void SystemdWidget::on_dependsClearButton_clicked()
{
    ui->dependenciesTableWidget->setRowCount(0);
}

void SystemdWidget::on_dependRemoveButton_clicked()
{
    auto *table = ui->dependenciesTableWidget;

    QSet<int> rows;
    for (const auto &index : table->selectionModel()->selectedRows())
    {
        rows.insert(index.row());
    }

    QList<int> rowsList = rows.values();
    std::sort(rowsList.begin(), rowsList.end(), std::greater<>());

    for (const int row : rowsList)
    {
        table->removeRow(row);
    }
}

} // namespace preferences
