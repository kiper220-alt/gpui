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

#include "cpuspeedwidget.h"
#include "ui_cpuspeedwidget.h"

namespace preferences
{

CpuSpeedFilterWidget::CpuSpeedFilterWidget(QWidget *parent)
    : TargetingFilterWidget(parent)
    , ui(new Ui::CPUSpeedWidget)
{
    ui->setupUi(this);
    ui->mhzComboBox->setEditable(true);
}

CpuSpeedFilterWidget::~CpuSpeedFilterWidget()
{
    delete ui;
}

QStringList CpuSpeedFilterWidget::knownKeys() const
{
    return {QStringLiteral("speedMHz")};
}

void CpuSpeedFilterWidget::readFromExtras(const QMap<QString, QString> &extras)
{
    const QString value = extras.value(QStringLiteral("speedMHz"));
    if (value.isEmpty())
    {
        return;
    }
    const int idx = ui->mhzComboBox->findText(value);
    if (idx >= 0)
    {
        ui->mhzComboBox->setCurrentIndex(idx);
    }
    else
    {
        ui->mhzComboBox->setEditText(value);
    }
}

QMap<QString, QString> CpuSpeedFilterWidget::writeToExtras() const
{
    return {{QStringLiteral("speedMHz"), ui->mhzComboBox->currentText()}};
}

} // namespace preferences
