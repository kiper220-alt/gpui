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

#include "operatingsystemwidget.h"
#include "ui_operatingsystemwidget.h"

#include <QStringList>

namespace preferences
{

namespace
{
const QStringList &osVersionEnum()
{
    static const QStringList v = {
        QStringLiteral("NE"),  QStringLiteral("95"),  QStringLiteral("98"),
        QStringLiteral("ME"),  QStringLiteral("NT"),  QStringLiteral("2K"),
        QStringLiteral("XP"),  QStringLiteral("2K3"), QStringLiteral("2K3R2"),
        QStringLiteral("VISTA"), QStringLiteral("2K8"), QStringLiteral("WIN7"),
        QStringLiteral("2K8R2"), QStringLiteral("WIN8"), QStringLiteral("WIN8S"),
        QStringLiteral("WINBLUE"), QStringLiteral("WINBLUESRV"),
        QStringLiteral("WINTHRESHOLD"), QStringLiteral("WINTHRESHOLDSRV"),
    };
    return v;
}

const QStringList &osEditionEnum()
{
    static const QStringList v = {
        QStringLiteral("NE"), QStringLiteral("64EP"), QStringLiteral("64DC"),
        QStringLiteral("AS"), QStringLiteral("DTC"),  QStringLiteral("EP"),
        QStringLiteral("WEB"), QStringLiteral("64"),  QStringLiteral("HM"),
        QStringLiteral("MC"),  QStringLiteral("TPC"), QStringLiteral("SRV"),
        QStringLiteral("STD"), QStringLiteral("TSE"), QStringLiteral("SBS"),
        QStringLiteral("PRO"),
    };
    return v;
}

const QStringList &osSpEnum()
{
    static const QStringList v = {
        QStringLiteral("NE"), QStringLiteral("Gold"),
        QStringLiteral("Service Pack 1"), QStringLiteral("Service Pack 2"),
        QStringLiteral("Service Pack 3"), QStringLiteral("Service Pack 4"),
        QStringLiteral("Service Pack 5"), QStringLiteral("Service Pack 6"),
    };
    return v;
}

const QStringList &osTypeEnum()
{
    static const QStringList v = {
        QStringLiteral("NE"), QStringLiteral("R2"), QStringLiteral("SE"),
        QStringLiteral("WS"), QStringLiteral("SV"), QStringLiteral("DC"),
        QStringLiteral("PRO"), QStringLiteral("PR"),
    };
    return v;
}

void selectByText(QComboBox *combo, const QString &text)
{
    if (text.isEmpty())
    {
        return;
    }
    const int idx = combo->findText(text);
    if (idx >= 0)
    {
        combo->setCurrentIndex(idx);
    }
}
} // namespace

OperatingSystemFilterWidget::OperatingSystemFilterWidget(QWidget *parent)
    : TargetingFilterWidget(parent)
    , ui(new Ui::OperatingSystemWidget)
{
    ui->setupUi(this);
    ui->productComboBox->addItems(osVersionEnum());
    ui->editionComboBox->addItems(osEditionEnum());
    ui->releaseComboBox->addItems(osSpEnum());
    ui->roleComboBox->addItems(osTypeEnum());
}

OperatingSystemFilterWidget::~OperatingSystemFilterWidget()
{
    delete ui;
}

QStringList OperatingSystemFilterWidget::knownKeys() const
{
    return {QStringLiteral("version"),
            QStringLiteral("edition"),
            QStringLiteral("sp"),
            QStringLiteral("type")};
}

void OperatingSystemFilterWidget::readFromExtras(const QMap<QString, QString> &extras)
{
    selectByText(ui->productComboBox, extras.value(QStringLiteral("version"), QStringLiteral("NE")));
    selectByText(ui->editionComboBox, extras.value(QStringLiteral("edition"), QStringLiteral("NE")));
    selectByText(ui->releaseComboBox, extras.value(QStringLiteral("sp"),      QStringLiteral("NE")));
    selectByText(ui->roleComboBox,    extras.value(QStringLiteral("type"),    QStringLiteral("NE")));
}

QMap<QString, QString> OperatingSystemFilterWidget::writeToExtras() const
{
    return {
        {QStringLiteral("version"), ui->productComboBox->currentText()},
        {QStringLiteral("edition"), ui->editionComboBox->currentText()},
        {QStringLiteral("sp"),      ui->releaseComboBox->currentText()},
        {QStringLiteral("type"),    ui->roleComboBox->currentText()},
    };
}

} // namespace preferences
