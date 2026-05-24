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

#include "domainfilterwidget.h"
#include "ui_domainwidget.h"

namespace preferences
{

DomainFilterWidget::DomainFilterWidget(QWidget *parent)
    : TargetingFilterWidget(parent)
    , ui(new Ui::DomainWidget)
{
    ui->setupUi(this);
    ui->toolButton->setEnabled(false);
    ui->toolButton->setToolTip(tr("Selection dialog is not implemented yet"));
}

DomainFilterWidget::~DomainFilterWidget()
{
    delete ui;
}

QStringList DomainFilterWidget::knownKeys() const
{
    return {QStringLiteral("name"), QStringLiteral("userContext")};
}

void DomainFilterWidget::readFromExtras(const QMap<QString, QString> &extras)
{
    ui->domainLineEdit->setText(extras.value(QStringLiteral("name")));
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
}

QMap<QString, QString> DomainFilterWidget::writeToExtras() const
{
    return {
        {QStringLiteral("name"), ui->domainLineEdit->text()},
        {QStringLiteral("userContext"),
         ui->userRadioButton->isChecked() ? QStringLiteral("1") : QStringLiteral("0")},
    };
}

} // namespace preferences
