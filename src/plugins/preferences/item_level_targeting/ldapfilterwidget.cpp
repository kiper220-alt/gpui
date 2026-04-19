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

#include "ldapfilterwidget.h"
#include "ui_ldapwidget.h"

namespace preferences
{

LdapFilterWidget::LdapFilterWidget(QWidget *parent)
    : TargetingFilterWidget(parent)
    , ui(new Ui::LDAPWidget)
{
    ui->setupUi(this);
}

LdapFilterWidget::~LdapFilterWidget()
{
    delete ui;
}

QStringList LdapFilterWidget::knownKeys() const
{
    return {QStringLiteral("searchFilter"),
            QStringLiteral("binding"),
            QStringLiteral("variableName"),
            QStringLiteral("attribute")};
}

void LdapFilterWidget::readFromExtras(const QMap<QString, QString> &extras)
{
    ui->filterLineEdit->setText(extras.value(QStringLiteral("searchFilter")));
    ui->bindingLineEdit->setText(extras.value(QStringLiteral("binding")));
    ui->varLineEdit->setText(extras.value(QStringLiteral("variableName")));
    ui->attrLineEdit->setText(extras.value(QStringLiteral("attribute")));
}

QMap<QString, QString> LdapFilterWidget::writeToExtras() const
{
    return {
        {QStringLiteral("searchFilter"), ui->filterLineEdit->text()},
        {QStringLiteral("binding"),      ui->bindingLineEdit->text()},
        {QStringLiteral("variableName"), ui->varLineEdit->text()},
        {QStringLiteral("attribute"),    ui->attrLineEdit->text()},
    };
}

} // namespace preferences
