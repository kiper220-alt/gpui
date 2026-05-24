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

#include "userfilterwidget.h"
#include "ui_userwidget.h"

namespace preferences
{

UserFilterWidget::UserFilterWidget(QWidget *parent)
    : TargetingFilterWidget(parent)
    , ui(new Ui::UserWidget)
{
    ui->setupUi(this);
    ui->usertoolButton->setEnabled(false);
    ui->usertoolButton->setToolTip(tr("Selection dialog is not implemented yet"));
}

UserFilterWidget::~UserFilterWidget()
{
    delete ui;
}

QStringList UserFilterWidget::knownKeys() const
{
    return {QStringLiteral("name"), QStringLiteral("sid")};
}

void UserFilterWidget::readFromExtras(const QMap<QString, QString> &extras)
{
    const QString sid  = extras.value(QStringLiteral("sid"));
    const QString name = extras.value(QStringLiteral("name"));
    if (!sid.isEmpty())
    {
        ui->sidCheckBox->setChecked(true);
        ui->userlineEdit->setText(sid);
    }
    else
    {
        ui->sidCheckBox->setChecked(false);
        ui->userlineEdit->setText(name);
    }
}

QMap<QString, QString> UserFilterWidget::writeToExtras() const
{
    if (ui->sidCheckBox->isChecked())
    {
        return {{QStringLiteral("sid"), ui->userlineEdit->text()}};
    }
    return {{QStringLiteral("name"), ui->userlineEdit->text()}};
}

} // namespace preferences
