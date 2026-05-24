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

#include "oscatalog.h"

#include <QComboBox>
#include <QSignalBlocker>

namespace preferences
{

namespace
{
QString currentValue(const QComboBox *combo)
{
    return combo->currentData().toString();
}

void addChoice(QComboBox *combo, const OsChoice &choice)
{
    combo->addItem(choice.label, choice.value);
}

void addUnknownValue(QComboBox *combo, const QString &value)
{
    if (!value.isEmpty() && combo->findData(value) < 0)
    {
        combo->addItem(value, value);
    }
}

void selectByValue(QComboBox *combo, const QString &value)
{
    if (value.isEmpty())
    {
        return;
    }
    const int idx = combo->findData(value);
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

    populateProducts();
    populateDependentCombos();

    connect(ui->productComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this]() { populateDependentCombos(); });
}

OperatingSystemFilterWidget::~OperatingSystemFilterWidget()
{
    delete ui;
}

QStringList OperatingSystemFilterWidget::knownKeys() const
{
    return {QStringLiteral("version"),
            QStringLiteral("class"),
            QStringLiteral("edition"),
            QStringLiteral("sp"),
            QStringLiteral("type")};
}

void OperatingSystemFilterWidget::readFromExtras(const QMap<QString, QString> &extras)
{
    m_osClass = extras.value(QStringLiteral("class"), QStringLiteral("NE"));
    const QString product = extras.value(QStringLiteral("version"), QStringLiteral("NE"));
    addUnknownValue(ui->productComboBox, product);
    selectByValue(ui->productComboBox, product);

    populateDependentCombos(extras.value(QStringLiteral("edition"), QStringLiteral("NE")),
                            extras.value(QStringLiteral("sp"),      QStringLiteral("NE")),
                            extras.value(QStringLiteral("type"),    QStringLiteral("NE")));
}

QMap<QString, QString> OperatingSystemFilterWidget::writeToExtras() const
{
    return {
        {QStringLiteral("class"),   m_osClass},
        {QStringLiteral("version"), currentValue(ui->productComboBox)},
        {QStringLiteral("edition"), currentValue(ui->editionComboBox)},
        {QStringLiteral("sp"),      currentValue(ui->releaseComboBox)},
        {QStringLiteral("type"),    currentValue(ui->roleComboBox)},
    };
}

void OperatingSystemFilterWidget::populateProducts()
{
    QSignalBlocker block(ui->productComboBox);
    ui->productComboBox->clear();
    for (const auto &product : OsCatalog::visibleProducts())
    {
        ui->productComboBox->addItem(product.label, product.value);
    }
}

void OperatingSystemFilterWidget::populateDependentCombos(const QString &editionValue,
                                                          const QString &servicePackValue,
                                                          const QString &roleValue)
{
    const QString productValue = currentValue(ui->productComboBox);
    const OsProduct product = OsCatalog::productByValue(productValue);

    const QString edition = editionValue.isEmpty() ? QStringLiteral("NE") : editionValue;
    const QString servicePack = servicePackValue.isEmpty() ? QStringLiteral("NE") : servicePackValue;
    const QString role = roleValue.isEmpty() ? QStringLiteral("NE") : roleValue;

    {
        QSignalBlocker block(ui->editionComboBox);
        ui->editionComboBox->clear();
        for (const auto &choice : product.editions)
        {
            addChoice(ui->editionComboBox, choice);
        }
        addUnknownValue(ui->editionComboBox, edition);
        selectByValue(ui->editionComboBox, edition);
    }

    {
        QSignalBlocker block(ui->releaseComboBox);
        ui->releaseComboBox->clear();
        for (const auto &choice : product.servicePacks)
        {
            addChoice(ui->releaseComboBox, choice);
        }
        addUnknownValue(ui->releaseComboBox, servicePack);
        selectByValue(ui->releaseComboBox, servicePack);
    }

    {
        QSignalBlocker block(ui->roleComboBox);
        ui->roleComboBox->clear();
        for (const auto &choice : product.roles)
        {
            addChoice(ui->roleComboBox, choice);
        }
        addUnknownValue(ui->roleComboBox, role);
        selectByValue(ui->roleComboBox, role);
    }
}

} // namespace preferences
