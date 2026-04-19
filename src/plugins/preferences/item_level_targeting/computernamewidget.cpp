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

#include "computernamewidget.h"
#include "ui_computernamewidget.h"

namespace preferences
{

ComputerNameFilterWidget::ComputerNameFilterWidget(QWidget *parent)
    : TargetingFilterWidget(parent)
    , ui(new Ui::computerNameWidget)
{
    ui->setupUi(this);
}

ComputerNameFilterWidget::~ComputerNameFilterWidget()
{
    delete ui;
}

QStringList ComputerNameFilterWidget::knownKeys() const
{
    return {QStringLiteral("name"), QStringLiteral("type")};
}

void ComputerNameFilterWidget::readFromExtras(const QMap<QString, QString> &extras)
{
    ui->nameLineEdit->setText(extras.value(QStringLiteral("name")));
    const QString type = extras.value(QStringLiteral("type"), QStringLiteral("NETBIOS"));
    if (type == QLatin1String("DNS"))
    {
        ui->dnsRadioButton->setChecked(true);
    }
    else
    {
        ui->biosRadioButton->setChecked(true);
    }
}

QMap<QString, QString> ComputerNameFilterWidget::writeToExtras() const
{
    return {
        {QStringLiteral("name"), ui->nameLineEdit->text()},
        {QStringLiteral("type"),
         ui->dnsRadioButton->isChecked() ? QStringLiteral("DNS") : QStringLiteral("NETBIOS")},
    };
}

} // namespace preferences
