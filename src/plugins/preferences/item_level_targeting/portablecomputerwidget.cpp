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

#include "portablecomputerwidget.h"
#include "ui_portablecomputerwidget.h"

namespace preferences
{

namespace
{
QString boolAttr(const QMap<QString, QString> &extras, const QString &key)
{
    return extras.value(key, QStringLiteral("0"));
}

bool boolFlag(const QMap<QString, QString> &extras, const QString &key)
{
    return boolAttr(extras, key) != QLatin1String("0");
}
} // namespace

PortableComputerFilterWidget::PortableComputerFilterWidget(QWidget *parent)
    : TargetingFilterWidget(parent)
    , ui(new Ui::PortableComputerWidget)
{
    ui->setupUi(this);
}

PortableComputerFilterWidget::~PortableComputerFilterWidget()
{
    delete ui;
}

QStringList PortableComputerFilterWidget::knownKeys() const
{
    return {QStringLiteral("unknown"), QStringLiteral("docked"), QStringLiteral("undocked")};
}

void PortableComputerFilterWidget::readFromExtras(const QMap<QString, QString> &extras)
{
    ui->noCheckBox->setChecked(boolFlag(extras, QStringLiteral("unknown")));
    ui->dockedCheckBox->setChecked(boolFlag(extras, QStringLiteral("docked")));
    ui->undockedCheckBox->setChecked(boolFlag(extras, QStringLiteral("undocked")));
}

QMap<QString, QString> PortableComputerFilterWidget::writeToExtras() const
{
    return {
        {QStringLiteral("unknown"),
         ui->noCheckBox->isChecked() ? QStringLiteral("1") : QStringLiteral("0")},
        {QStringLiteral("docked"),
         ui->dockedCheckBox->isChecked() ? QStringLiteral("1") : QStringLiteral("0")},
        {QStringLiteral("undocked"),
         ui->undockedCheckBox->isChecked() ? QStringLiteral("1") : QStringLiteral("0")},
    };
}

} // namespace preferences
