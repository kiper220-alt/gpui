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

#include "processingmodewidget.h"
#include "ui_processingmodewidget.h"

namespace preferences
{

namespace
{
QString boolStr(bool checked)
{
    return checked ? QStringLiteral("1") : QStringLiteral("0");
}

bool boolFlag(const QMap<QString, QString> &extras, const QString &key)
{
    return extras.value(key, QStringLiteral("0")) != QLatin1String("0");
}
} // namespace

ProcessingModeFilterWidget::ProcessingModeFilterWidget(QWidget *parent)
    : TargetingFilterWidget(parent)
    , ui(new Ui::ProcessingModeWidget)
{
    ui->setupUi(this);
}

ProcessingModeFilterWidget::~ProcessingModeFilterWidget()
{
    delete ui;
}

QStringList ProcessingModeFilterWidget::knownKeys() const
{
    return {
        QStringLiteral("synchFore"), QStringLiteral("asynchFore"),
        QStringLiteral("backRefr"),  QStringLiteral("forceRefr"),
        QStringLiteral("linkTrns"),  QStringLiteral("noChg"),
        QStringLiteral("rsopTrns"),  QStringLiteral("safeBoot"),
        QStringLiteral("slowLink"),  QStringLiteral("verbLog"),
    };
}

void ProcessingModeFilterWidget::readFromExtras(const QMap<QString, QString> &extras)
{
    ui->syncCheckBox->setChecked(boolFlag(extras, QStringLiteral("synchFore")));
    ui->asyncCheckBox->setChecked(boolFlag(extras, QStringLiteral("asynchFore")));
    ui->backCheckBox->setChecked(boolFlag(extras, QStringLiteral("backRefr")));
    ui->forceCheckBox->setChecked(boolFlag(extras, QStringLiteral("forceRefr")));
    ui->transCheckBox->setChecked(boolFlag(extras, QStringLiteral("linkTrns")));
    ui->nochangeCheckBox->setChecked(boolFlag(extras, QStringLiteral("noChg")));
    ui->rsopCheckBox->setChecked(boolFlag(extras, QStringLiteral("rsopTrns")));
    ui->safeCheckBox->setChecked(boolFlag(extras, QStringLiteral("safeBoot")));
    ui->slowCheckBox->setChecked(boolFlag(extras, QStringLiteral("slowLink")));
    ui->logCheckBox->setChecked(boolFlag(extras, QStringLiteral("verbLog")));
}

QMap<QString, QString> ProcessingModeFilterWidget::writeToExtras() const
{
    return {
        {QStringLiteral("synchFore"),  boolStr(ui->syncCheckBox->isChecked())},
        {QStringLiteral("asynchFore"), boolStr(ui->asyncCheckBox->isChecked())},
        {QStringLiteral("backRefr"),   boolStr(ui->backCheckBox->isChecked())},
        {QStringLiteral("forceRefr"),  boolStr(ui->forceCheckBox->isChecked())},
        {QStringLiteral("linkTrns"),   boolStr(ui->transCheckBox->isChecked())},
        {QStringLiteral("noChg"),      boolStr(ui->nochangeCheckBox->isChecked())},
        {QStringLiteral("rsopTrns"),   boolStr(ui->rsopCheckBox->isChecked())},
        {QStringLiteral("safeBoot"),   boolStr(ui->safeCheckBox->isChecked())},
        {QStringLiteral("slowLink"),   boolStr(ui->slowCheckBox->isChecked())},
        {QStringLiteral("verbLog"),    boolStr(ui->logCheckBox->isChecked())},
    };
}

} // namespace preferences
