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

#include "wmiquerywidget.h"
#include "ui_wmiquerywidget.h"

namespace preferences
{

WmiQueryFilterWidget::WmiQueryFilterWidget(QWidget *parent)
    : TargetingFilterWidget(parent)
    , ui(new Ui::WmiQueryWidget)
{
    ui->setupUi(this);

    connect(ui->propLineEdit, &QLineEdit::textChanged,
            this, &WmiQueryFilterWidget::onPropertyChanged);
}

WmiQueryFilterWidget::~WmiQueryFilterWidget()
{
    delete ui;
}

QStringList WmiQueryFilterWidget::knownKeys() const
{
    return {QStringLiteral("query"), QStringLiteral("nameSpace"),
            QStringLiteral("property"), QStringLiteral("variableName")};
}

void WmiQueryFilterWidget::readFromExtras(const QMap<QString, QString> &extras)
{
    ui->queryLineEdit->setText(extras.value(QStringLiteral("query")));
    if (extras.contains(QStringLiteral("nameSpace")))
    {
        ui->nameLineEdit->setText(extras.value(QStringLiteral("nameSpace")));
    }
    ui->propLineEdit->setText(extras.value(QStringLiteral("property")));
    ui->envLineEdit->setText(extras.value(QStringLiteral("variableName")));
}

QMap<QString, QString> WmiQueryFilterWidget::writeToExtras() const
{
    return {
        {QStringLiteral("query"),        ui->queryLineEdit->text()},
        {QStringLiteral("nameSpace"),    ui->nameLineEdit->text()},
        {QStringLiteral("property"),     ui->propLineEdit->text()},
        {QStringLiteral("variableName"), ui->envLineEdit->text()},
    };
}

void WmiQueryFilterWidget::onPropertyChanged(const QString &text)
{
    const bool hasProperty = !text.trimmed().isEmpty();
    ui->envLineEdit->setEnabled(hasProperty);
    ui->envLabel->setEnabled(hasProperty);
}

} // namespace preferences
