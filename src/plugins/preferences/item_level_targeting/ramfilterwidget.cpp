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

#include "ramfilterwidget.h"
#include "ui_ramwidget.h"

namespace preferences
{

RamFilterWidget::RamFilterWidget(QWidget *parent)
    : TargetingFilterWidget(parent)
    , ui(new Ui::RamWidget)
{
    ui->setupUi(this);
    ui->mbComboBox->setEditable(true);
}

RamFilterWidget::~RamFilterWidget()
{
    delete ui;
}

QStringList RamFilterWidget::knownKeys() const
{
    return {QStringLiteral("totalMB")};
}

void RamFilterWidget::readFromExtras(const QMap<QString, QString> &extras)
{
    const QString value = extras.value(QStringLiteral("totalMB"));
    if (value.isEmpty())
    {
        return;
    }
    const int idx = ui->mbComboBox->findText(value);
    if (idx >= 0)
    {
        ui->mbComboBox->setCurrentIndex(idx);
    }
    else
    {
        ui->mbComboBox->setEditText(value);
    }
}

QMap<QString, QString> RamFilterWidget::writeToExtras() const
{
    return {{QStringLiteral("totalMB"), ui->mbComboBox->currentText()}};
}

} // namespace preferences
