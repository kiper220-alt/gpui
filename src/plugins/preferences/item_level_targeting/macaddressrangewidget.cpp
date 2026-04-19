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

#include "macaddressrangewidget.h"
#include "ui_macaddressrangewidget.h"

namespace preferences
{

MacAddressRangeFilterWidget::MacAddressRangeFilterWidget(QWidget *parent)
    : TargetingFilterWidget(parent)
    , ui(new Ui::MacAdderessRangeWidget)
{
    ui->setupUi(this);
}

MacAddressRangeFilterWidget::~MacAddressRangeFilterWidget()
{
    delete ui;
}

QStringList MacAddressRangeFilterWidget::knownKeys() const
{
    return {QStringLiteral("min"), QStringLiteral("max")};
}

void MacAddressRangeFilterWidget::readFromExtras(const QMap<QString, QString> &extras)
{
    ui->betweenLineEdit->setText(extras.value(QStringLiteral("min")));
    ui->andLineEdit->setText(extras.value(QStringLiteral("max")));
}

QMap<QString, QString> MacAddressRangeFilterWidget::writeToExtras() const
{
    return {
        {QStringLiteral("min"), ui->betweenLineEdit->text()},
        {QStringLiteral("max"), ui->andLineEdit->text()},
    };
}

} // namespace preferences
