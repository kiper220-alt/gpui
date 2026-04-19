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

#include "organizationunitwidget.h"
#include "ui_organizationuniitwidget.h"

namespace preferences
{

OrganizationUnitFilterWidget::OrganizationUnitFilterWidget(QWidget *parent)
    : TargetingFilterWidget(parent)
    , ui(new Ui::OrganizationUnitWidget)
{
    ui->setupUi(this);
}

OrganizationUnitFilterWidget::~OrganizationUnitFilterWidget()
{
    delete ui;
}

QStringList OrganizationUnitFilterWidget::knownKeys() const
{
    return {QStringLiteral("name"), QStringLiteral("userContext"), QStringLiteral("directMember")};
}

void OrganizationUnitFilterWidget::readFromExtras(const QMap<QString, QString> &extras)
{
    ui->unitLineEdit->setText(extras.value(QStringLiteral("name")));
    const bool userCtx = extras.value(QStringLiteral("userContext"), QStringLiteral("1"))
                         != QLatin1String("0");
    if (userCtx)
    {
        ui->userRadioButton->setChecked(true);
    }
    else
    {
        ui->computerRadioButton->setChecked(true);
    }
    ui->onlyCheckBox->setChecked(extras.value(QStringLiteral("directMember"), QStringLiteral("0"))
                                 != QLatin1String("0"));
}

QMap<QString, QString> OrganizationUnitFilterWidget::writeToExtras() const
{
    return {
        {QStringLiteral("name"), ui->unitLineEdit->text()},
        {QStringLiteral("userContext"),
         ui->userRadioButton->isChecked() ? QStringLiteral("1") : QStringLiteral("0")},
        {QStringLiteral("directMember"),
         ui->onlyCheckBox->isChecked() ? QStringLiteral("1") : QStringLiteral("0")},
    };
}

} // namespace preferences
