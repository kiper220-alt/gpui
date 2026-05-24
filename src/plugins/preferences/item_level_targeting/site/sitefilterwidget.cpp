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

#include "sitefilterwidget.h"
#include "ui_sitewidget.h"

namespace preferences
{

SiteFilterWidget::SiteFilterWidget(QWidget *parent)
    : TargetingFilterWidget(parent)
    , ui(new Ui::SiteWidget)
{
    ui->setupUi(this);
    ui->siteToolButton->setEnabled(false);
    ui->siteToolButton->setToolTip(tr("Selection dialog is not implemented yet"));
}

SiteFilterWidget::~SiteFilterWidget()
{
    delete ui;
}

QStringList SiteFilterWidget::knownKeys() const
{
    return {QStringLiteral("name")};
}

void SiteFilterWidget::readFromExtras(const QMap<QString, QString> &extras)
{
    ui->siteLineEdit->setText(extras.value(QStringLiteral("name")));
}

QMap<QString, QString> SiteFilterWidget::writeToExtras() const
{
    return {{QStringLiteral("name"), ui->siteLineEdit->text()}};
}

} // namespace preferences
