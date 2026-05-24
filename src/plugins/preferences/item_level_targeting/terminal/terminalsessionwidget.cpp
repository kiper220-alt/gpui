/***********************************************************************************************************************
**
** Copyright (C) 2026 BaseALT Ltd. <org@basealt.ru>
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation
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

#include "terminalsessionwidget.h"
#include "ui_terminalsessionwidget.h"

#include <QRegularExpressionValidator>

namespace preferences
{

namespace
{
const QString kType   = QStringLiteral("type");
const QString kOption = QStringLiteral("option");
const QString kValue  = QStringLiteral("value");
const QString kMin    = QStringLiteral("min");
const QString kMax    = QStringLiteral("max");

const QStringList optionOrder = {
    QStringLiteral("NE"),
    QStringLiteral("APPLICATION"),
    QStringLiteral("CLIENT"),
    QStringLiteral("PROGRAM"),
    QStringLiteral("SESSION"),
    QStringLiteral("DIRECTORY"),
    QStringLiteral("IP"),
};

const QStringList typeOrder = {
    QStringLiteral("NE"),
    QStringLiteral("TS"),
    QStringLiteral("CONSOLE"),
};

QRegularExpression ipRegex()
{
    return QRegularExpression(QStringLiteral(
        "^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}"
        "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$"));
}
} // namespace

TerminalSessionFilterWidget::TerminalSessionFilterWidget(QWidget *parent)
    : TargetingFilterWidget(parent)
    , ui(new Ui::TerminalSessionWidget)
{
    ui->setupUi(this);

    auto *ipValidator = new QRegularExpressionValidator(ipRegex(), this);
    ui->minLineEdit->setValidator(ipValidator);
    ui->maxLineEdit->setValidator(new QRegularExpressionValidator(ipRegex(), this));

    connect(ui->paramComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TerminalSessionFilterWidget::onParamChanged);

    onParamChanged(0);
}

TerminalSessionFilterWidget::~TerminalSessionFilterWidget()
{
    delete ui;
}

QStringList TerminalSessionFilterWidget::knownKeys() const
{
    return {kType, kOption, kValue, kMin, kMax};
}

void TerminalSessionFilterWidget::onParamChanged(int index)
{
    const bool hasValue = (index > 0);
    const bool isIpRange = (index == 6);
    ui->paramValLineEdit->setEnabled(hasValue);
    ui->paramValLabel->setEnabled(hasValue);
    ui->rangeLabel->setEnabled(isIpRange);
    ui->rangeAndLabel->setEnabled(isIpRange);
    ui->minLineEdit->setEnabled(isIpRange);
    ui->maxLineEdit->setEnabled(isIpRange);

    if (isIpRange) // Client TCP/IP address
    {
        ui->paramValLineEdit->setValidator(new QRegularExpressionValidator(ipRegex(), this));
    }
    else
    {
        ui->paramValLineEdit->setValidator(nullptr);
    }
}

void TerminalSessionFilterWidget::readFromExtras(const QMap<QString, QString> &extras)
{
    const int typeIdx = typeOrder.indexOf(extras.value(kType));
    if (typeIdx >= 0 && typeIdx < ui->typeComboBox->count())
    {
        ui->typeComboBox->setCurrentIndex(typeIdx);
    }

    const int optIdx = optionOrder.indexOf(extras.value(kOption));
    if (optIdx >= 0 && optIdx < ui->paramComboBox->count())
    {
        ui->paramComboBox->setCurrentIndex(optIdx);
    }

    ui->paramValLineEdit->setText(extras.value(kValue));
    ui->minLineEdit->setText(extras.value(kMin, QStringLiteral("0.0.0.0")));
    ui->maxLineEdit->setText(extras.value(kMax, QStringLiteral("0.0.0.0")));
}

QMap<QString, QString> TerminalSessionFilterWidget::writeToExtras() const
{
    QMap<QString, QString> result;

    const int typeIdx = ui->typeComboBox->currentIndex();
    if (typeIdx >= 0 && typeIdx < typeOrder.size())
    {
        result[kType] = typeOrder.at(typeIdx);
    }

    const int optIdx = ui->paramComboBox->currentIndex();
    if (optIdx >= 0 && optIdx < optionOrder.size())
    {
        result[kOption] = optionOrder.at(optIdx);
    }

    result[kValue] = ui->paramValLineEdit->text();
    if (result.value(kOption) == QLatin1String("IP"))
    {
        result[kMin] = ui->minLineEdit->text();
        result[kMax] = ui->maxLineEdit->text();
    }

    return result;
}

} // namespace preferences
