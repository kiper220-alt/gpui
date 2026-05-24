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

#include "environmentvariablewidget.h"
#include "ui_environmentwidget.h"

namespace preferences
{

EnvironmentVariableFilterWidget::EnvironmentVariableFilterWidget(QWidget *parent)
    : TargetingFilterWidget(parent)
    , ui(new Ui::EnvironmentWidget)
{
    ui->setupUi(this);
}

EnvironmentVariableFilterWidget::~EnvironmentVariableFilterWidget()
{
    delete ui;
}

QStringList EnvironmentVariableFilterWidget::knownKeys() const
{
    return {QStringLiteral("variableName"), QStringLiteral("value")};
}

void EnvironmentVariableFilterWidget::readFromExtras(const QMap<QString, QString> &extras)
{
    ui->domainLineEdit->setText(extras.value(QStringLiteral("variableName")));
    ui->valueLineEdit->setText(extras.value(QStringLiteral("value")));
}

QMap<QString, QString> EnvironmentVariableFilterWidget::writeToExtras() const
{
    return {
        {QStringLiteral("variableName"), ui->domainLineEdit->text()},
        {QStringLiteral("value"), ui->valueLineEdit->text()},
    };
}

} // namespace preferences
