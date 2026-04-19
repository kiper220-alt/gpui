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

#include "languagewidget.h"
#include "ui_languagewidget.h"

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

LanguageFilterWidget::LanguageFilterWidget(QWidget *parent)
    : TargetingFilterWidget(parent)
    , ui(new Ui::LanguageWidget)
{
    ui->setupUi(this);
    ui->languageComboBox->setEditable(true);
}

LanguageFilterWidget::~LanguageFilterWidget()
{
    delete ui;
}

QStringList LanguageFilterWidget::knownKeys() const
{
    return {QStringLiteral("default"),
            QStringLiteral("system"),
            QStringLiteral("native"),
            QStringLiteral("displayName")};
}

void LanguageFilterWidget::readFromExtras(const QMap<QString, QString> &extras)
{
    ui->userCheckBox->setChecked(boolFlag(extras, QStringLiteral("default")));
    ui->systemCheckBox->setChecked(boolFlag(extras, QStringLiteral("system")));
    ui->nativeCheckBox->setChecked(boolFlag(extras, QStringLiteral("native")));
    const QString displayName = extras.value(QStringLiteral("displayName"));
    if (!displayName.isEmpty())
    {
        const int idx = ui->languageComboBox->findText(displayName);
        if (idx >= 0)
        {
            ui->languageComboBox->setCurrentIndex(idx);
        }
        else
        {
            ui->languageComboBox->setEditText(displayName);
        }
    }
}

QMap<QString, QString> LanguageFilterWidget::writeToExtras() const
{
    return {
        {QStringLiteral("default"),     boolStr(ui->userCheckBox->isChecked())},
        {QStringLiteral("system"),      boolStr(ui->systemCheckBox->isChecked())},
        {QStringLiteral("native"),      boolStr(ui->nativeCheckBox->isChecked())},
        {QStringLiteral("displayName"), ui->languageComboBox->currentText()},
    };
}

} // namespace preferences
