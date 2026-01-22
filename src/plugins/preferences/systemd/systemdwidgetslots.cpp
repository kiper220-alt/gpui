/***********************************************************************************************************************
**
** Copyright (C) 2025 BaseALT Ltd. <org@basealt.ru>
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

#include "systemditem.h"
#include "systemdwidget.h"
#include "ui_systemdwidget.h"
#include "common/commonitem.h"

namespace preferences 
{

/**
 * @brief Submits changes to the item
 *
 * If the mapper is valid and the data is valid, this function
 * will submit the changes to the mapper and update the item's
 * state and state_now properties. If the state checkbox is
 * checked, the state property will be set to the current index
 * of the state combo box plus 1. If the now checkbox is checked,
 * the state_now property will be set to true. The function will
 * then write the changes to the edit table and dependency table and
 * emit a data changed signal.
 */
void SystemdWidget::submit()
{
    if (mapper && validate()) 
    {
        mapper->submit();

        if (m_item)
        {
            m_item->setProperty(
                preferences::SystemdItem::STATE,
                ui->changeUnitStateCheckBox->isChecked() ? ui->changeUnitStateComboBox->currentIndex() + 1 : 0
            );
            m_item->setProperty(
                preferences::SystemdItem::STATE_NOW,
                ui->changeUnitStateCheckBox->isChecked() ? ui->nowCheckBox->isChecked() : false
            );

            this->writeEditTable();
            this->writeDependencyTable();
        }

        emit dataChanged();
    }
}

/**
 * @brief Creates a combo box widget with the following items: Create, Replace, update, delete
 *
 * @param parent The parent widget for the combo box
 * @return The created combo box widget
 */
static QWidget* createActionComboBox(QWidget* parent)
{
    const auto comboBox = new QComboBox(parent);
    comboBox->addItem("Create");
    comboBox->addItem("Replace");
    comboBox->addItem("Update");
    comboBox->addItem("Delete");
    return comboBox;
}

/**
 * @brief Creates a combo box widget with the following items: Changed, Presence Changed
 *
 * @param parent The parent widget for the combo box
 * @return The created combo box widget
 */
static QWidget* createDependenceComboBox(QWidget* parent)
{
    const auto comboBox = new QComboBox(parent);
    comboBox->addItem("Changed");
    comboBox->addItem("Presence Changed");
    return comboBox;
}

/**
 * @brief Adds a new row to the actions table with a combo box widget in the first column
 *
 * The combo box widget has the following items: Create, Replace, update, delete
 */
void SystemdWidget::on_actionAddButton_clicked() const
{
    const auto table = this->ui->actionsTableWidget;
    const int rows = table->rowCount();
    table->insertRow(rows);
    table->setCellWidget(rows, 0, createActionComboBox(table));
}
/**
 * @brief Clears the actions table by removing all rows.
 */
void SystemdWidget::on_actionsClearButton_clicked()
{
    const auto table = this->ui->actionsTableWidget;
    table->setRowCount(0);
}
/**
 * @brief Removes all selected rows from the actions table
 */
void SystemdWidget::on_actionRemoveButton_clicked()
{
    auto* table = ui->actionsTableWidget;

    QSet<int> rows;
    for (const auto& index : table->selectionModel()->selectedRows())
    {
        rows.insert(index.row());
    }

    QList<int> rowsList = rows.values();
    std::sort(rowsList.begin(), rowsList.end(), std::greater<>());

    for (int row : rowsList)
    {
        table->removeRow(row);
    }
}
/**
 * @brief Adds a new row to the dependencies table with a combo box widget in the first column
 *
 * The combo box widget has the following items: Changed, Presence Changed
 */
void SystemdWidget::on_dependAddButton_clicked()
{
    const auto table = this->ui->dependenciesTableWidget;
    const int rows = table->rowCount();
    table->insertRow(rows);
    table->setCellWidget(rows, 0, createDependenceComboBox(table));
}
/**
 * @brief Clears the dependencies table by removing all rows.
 */
void SystemdWidget::on_dependsClearButton_clicked()
{
    const auto table = this->ui->dependenciesTableWidget;
    table->setRowCount(0);
}
/**
 * @brief Removes all selected rows from the dependencies table.
 *
 * This function removes all the rows which are currently selected in the dependencies
 * table. The rows are removed in reverse order to prevent any indexing issues.
 */
void SystemdWidget::on_dependRemoveButton_clicked()
{
    const auto table = this->ui->dependenciesTableWidget;

    QSet<int> rows;
    for (const auto& index : table->selectionModel()->selectedRows())
    {
        rows.insert(index.row());
    }

    QList<int> rowsList = rows.values();
    std::sort(rowsList.begin(), rowsList.end(), std::greater<>());

    for (int row : rowsList)
    {
        table->removeRow(row);
    }
}

} // namespace