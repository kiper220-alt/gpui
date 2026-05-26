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

#include "securitygroupwidget.h"
#include "ui_securitygroupwidget.h"

namespace preferences
{

SecurityGroupFilterWidget::SecurityGroupFilterWidget(QWidget *parent)
    : TargetingFilterWidget(parent)
    , ui(new Ui::SecurityGroupWidget)
    , m_localGroup()
{
    ui->setupUi(this);
    ui->groupToolButton->setEnabled(false);
    ui->groupToolButton->setToolTip(tr("Selection dialog is not implemented yet"));
}

SecurityGroupFilterWidget::~SecurityGroupFilterWidget()
{
    delete ui;
}

QStringList SecurityGroupFilterWidget::knownKeys() const
{
    return {QStringLiteral("name"),
            QStringLiteral("sid"),
            QStringLiteral("userContext"),
            QStringLiteral("primaryGroup"),
            QStringLiteral("localGroup")};
}

void SecurityGroupFilterWidget::readFromExtras(const QMap<QString, QString> &extras)
{
    const QString sid  = extras.value(QStringLiteral("sid"));
    const QString name = extras.value(QStringLiteral("name"));
    ui->droupLineEdit->setText(sid.isEmpty() ? name : sid);

    const bool userCtx = extras.value(QStringLiteral("userContext"), QStringLiteral("1"))
                         != QLatin1String("0");
    if (userCtx)
    {
        ui->userRadioButton->setChecked(true);
    }
    else
    {
        ui->compRadioButton->setChecked(true);
    }
    ui->primaryCheckBox->setChecked(extras.value(QStringLiteral("primaryGroup"),
                                                 QStringLiteral("0"))
                                    != QLatin1String("0"));
    m_localGroup = extras.value(QStringLiteral("localGroup"), QStringLiteral("0"));
}

QMap<QString, QString> SecurityGroupFilterWidget::writeToExtras() const
{
    return {
        {QStringLiteral("name"),         ui->droupLineEdit->text()},
        {QStringLiteral("sid"),          QString()},
        {QStringLiteral("userContext"),
         ui->userRadioButton->isChecked() ? QStringLiteral("1") : QStringLiteral("0")},
        {QStringLiteral("primaryGroup"),
         ui->primaryCheckBox->isChecked() ? QStringLiteral("1") : QStringLiteral("0")},
        {QStringLiteral("localGroup"), m_localGroup},
    };
}

} // namespace preferences
