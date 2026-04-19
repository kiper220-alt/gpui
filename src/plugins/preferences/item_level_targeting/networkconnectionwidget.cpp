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

#include "networkconnectionwidget.h"
#include "ui_networkconnectionwidget.h"

#include <QStringList>

namespace preferences
{

namespace
{
//! XSD `enumFilterDunType` values, in the order the combo box presents them.
const QStringList &dunTypeOrder()
{
    static const QStringList order = {
        QStringLiteral(""),         // Any
        QStringLiteral("modem"),
        QStringLiteral("isdn"),
        QStringLiteral("x25"),
        QStringLiteral("vpn"),
        QStringLiteral("pad"),
        QStringLiteral("GENERIC"),
        QStringLiteral("SERIAL"),
        QStringLiteral("FRAMERELAY"),
        QStringLiteral("ATM"),
        QStringLiteral("SONET"),
        QStringLiteral("SW56"),
        QStringLiteral("IRDA"),
        QStringLiteral("PARALLEL"),
        QStringLiteral("PPPoE"),
    };
    return order;
}
} // namespace

NetworkConnectionFilterWidget::NetworkConnectionFilterWidget(QWidget *parent)
    : TargetingFilterWidget(parent)
    , ui(new Ui::NetworkConnectionWidget)
{
    ui->setupUi(this);
}

NetworkConnectionFilterWidget::~NetworkConnectionFilterWidget()
{
    delete ui;
}

QStringList NetworkConnectionFilterWidget::knownKeys() const
{
    return {QStringLiteral("type")};
}

void NetworkConnectionFilterWidget::readFromExtras(const QMap<QString, QString> &extras)
{
    const int idx = dunTypeOrder().indexOf(extras.value(QStringLiteral("type")));
    if (idx >= 0 && idx < ui->typeComboBox->count())
    {
        ui->typeComboBox->setCurrentIndex(idx);
    }
}

QMap<QString, QString> NetworkConnectionFilterWidget::writeToExtras() const
{
    const int idx = ui->typeComboBox->currentIndex();
    const QString value = (idx >= 0 && idx < dunTypeOrder().size()) ? dunTypeOrder().at(idx)
                                                                    : QString();
    return {{QStringLiteral("type"), value}};
}

} // namespace preferences
