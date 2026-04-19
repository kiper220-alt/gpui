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

#include "diskspacewidget.h"
#include "ui_diskspacewidget.h"

namespace preferences
{

namespace
{
//! Update an editable QComboBox to display `value`, switching the selection
//! to a matching item if one exists, otherwise dropping the literal text in.
void selectOrType(QComboBox *combo, const QString &value)
{
    if (value.isEmpty())
    {
        return;
    }
    const int idx = combo->findText(value);
    if (idx >= 0)
    {
        combo->setCurrentIndex(idx);
    }
    else
    {
        combo->setEditText(value);
    }
}
} // namespace

DiskSpaceFilterWidget::DiskSpaceFilterWidget(QWidget *parent)
    : TargetingFilterWidget(parent)
    , ui(new Ui::DiskSpaceWidget)
{
    ui->setupUi(this);
    ui->periodicComboBox->setEditable(true);
    ui->comboBox->setEditable(true);
}

DiskSpaceFilterWidget::~DiskSpaceFilterWidget()
{
    delete ui;
}

QStringList DiskSpaceFilterWidget::knownKeys() const
{
    return {QStringLiteral("freeSpace"), QStringLiteral("drive")};
}

void DiskSpaceFilterWidget::readFromExtras(const QMap<QString, QString> &extras)
{
    selectOrType(ui->periodicComboBox, extras.value(QStringLiteral("freeSpace")));
    selectOrType(ui->comboBox,         extras.value(QStringLiteral("drive")));
}

QMap<QString, QString> DiskSpaceFilterWidget::writeToExtras() const
{
    return {
        {QStringLiteral("freeSpace"), ui->periodicComboBox->currentText()},
        {QStringLiteral("drive"),     ui->comboBox->currentText()},
    };
}

} // namespace preferences
