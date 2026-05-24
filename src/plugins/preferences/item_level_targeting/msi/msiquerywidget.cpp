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

#include "msiquerywidget.h"
#include "ui_msiquerywidget.h"

namespace preferences
{

namespace
{
const QString kType         = QStringLiteral("type");
const QString kSubType      = QStringLiteral("subtype");
const QString kCode         = QStringLiteral("code");
const QString kValue        = QStringLiteral("value");
const QString kItem         = QStringLiteral("item");
const QString kMin          = QStringLiteral("min");
const QString kMax          = QStringLiteral("max");
const QString kGte          = QStringLiteral("gte");
const QString kLte          = QStringLiteral("lte");

const QStringList subtypeOrder = {
    QStringLiteral("EXISTS"),
    QStringLiteral("VERSION"),
    QStringLiteral("GET_PROPERTY"),
    QStringLiteral("MATCH_PROPERTY"),
    QStringLiteral("GET_INFORMATION"),
    QStringLiteral("MATCH_INFORMATION"),
};

const QStringList targetTypeOrder = {
    QStringLiteral("PRODUCT"),
    QStringLiteral("PATCH"),
    QStringLiteral("FILECOMPONENT"),
};
} // namespace

MsiQueryFilterWidget::MsiQueryFilterWidget(QWidget *parent)
    : TargetingFilterWidget(parent)
    , ui(new Ui::MSIQueryWidget)
{
    ui->setupUi(this);

    ui->productToolButton->setEnabled(false);
    ui->productToolButton->setToolTip(tr("Selection dialog is not implemented yet"));

    connect(ui->queryComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MsiQueryFilterWidget::onQueryTypeChanged);

    updateFieldVisibility(0);
}

MsiQueryFilterWidget::~MsiQueryFilterWidget()
{
    delete ui;
}

QStringList MsiQueryFilterWidget::knownKeys() const
{
    return {kType, kSubType, kCode, kValue, kItem, kMin, kMax, kGte, kLte};
}

void MsiQueryFilterWidget::onQueryTypeChanged(int index)
{
    updateFieldVisibility(index);
}

void MsiQueryFilterWidget::updateFieldVisibility(int queryType)
{
    ui->propertyNameLabel->setVisible(false);
    ui->propertyNameLineEdit->setVisible(false);
    ui->propertyValueLabel->setVisible(false);
    ui->propertyValueLineEdit->setVisible(false);
    ui->infoItemLabel->setVisible(false);
    ui->infoItemComboBox->setVisible(false);
    ui->infoValueLabel->setVisible(false);
    ui->infoValueLineEdit->setVisible(false);
    ui->variableNameLabel->setVisible(false);
    ui->variableNameLineEdit->setVisible(false);
    ui->minComboBox->setVisible(false);
    ui->minLineEdit->setVisible(false);
    ui->maxComboBox->setVisible(false);
    ui->maxLineEdit->setVisible(false);

    switch (queryType)
    {
    case 0: // EXISTS
        break;
    case 1: // VERSION
        ui->minComboBox->setVisible(true);
        ui->minLineEdit->setVisible(true);
        ui->maxComboBox->setVisible(true);
        ui->maxLineEdit->setVisible(true);
        break;
    case 2: // GET_PROPERTY
        ui->propertyNameLabel->setVisible(true);
        ui->propertyNameLineEdit->setVisible(true);
        ui->variableNameLabel->setVisible(true);
        ui->variableNameLineEdit->setVisible(true);
        break;
    case 3: // MATCH_PROPERTY
        ui->propertyNameLabel->setVisible(true);
        ui->propertyNameLineEdit->setVisible(true);
        ui->propertyValueLabel->setVisible(true);
        ui->propertyValueLineEdit->setVisible(true);
        break;
    case 4: // GET_INFORMATION
        ui->infoItemLabel->setVisible(true);
        ui->infoItemComboBox->setVisible(true);
        ui->variableNameLabel->setVisible(true);
        ui->variableNameLineEdit->setVisible(true);
        break;
    case 5: // MATCH_INFORMATION
        ui->infoItemLabel->setVisible(true);
        ui->infoItemComboBox->setVisible(true);
        ui->infoValueLabel->setVisible(true);
        ui->infoValueLineEdit->setVisible(true);
        break;
    }
}

void MsiQueryFilterWidget::readFromExtras(const QMap<QString, QString> &extras)
{
    const int targetIdx = targetTypeOrder.indexOf(extras.value(kType));
    if (targetIdx >= 0)
    {
        ui->targetComboBox->setCurrentIndex(targetIdx);
    }

    const int subIdx = subtypeOrder.indexOf(extras.value(kSubType));
    const int queryIdx = (subIdx >= 0) ? subIdx : 0;
    ui->queryComboBox->setCurrentIndex(queryIdx);

    ui->productLineEdit->setText(extras.value(kCode));
    if (queryIdx == 2 || queryIdx == 3)
    {
        ui->propertyNameLineEdit->setText(extras.value(kItem));
    }
    ui->propertyValueLineEdit->setText(extras.value(kValue));

    const QString item = extras.value(kItem);
    const int infoIdx = ui->infoItemComboBox->findText(item);
    if (infoIdx >= 0)
    {
        ui->infoItemComboBox->setCurrentIndex(infoIdx);
    }

    if (queryIdx == 2 || queryIdx == 4)
    {
        ui->variableNameLineEdit->setText(extras.value(kValue));
    }
    ui->infoValueLineEdit->setText(extras.value(kValue));

    if (queryIdx == 1)
    {
        ui->minLineEdit->setText(extras.value(kMin, QStringLiteral("0.0.0.0")));
        ui->maxLineEdit->setText(extras.value(kMax, QStringLiteral("0.0.0.0")));
        const bool gte = extras.value(kGte, QStringLiteral("0")) != QLatin1String("0");
        ui->minComboBox->setCurrentIndex(gte ? 0 : 1);
        const bool lte = extras.value(kLte, QStringLiteral("0")) != QLatin1String("0");
        ui->maxComboBox->setCurrentIndex(lte ? 1 : 0);
    }
}

QMap<QString, QString> MsiQueryFilterWidget::writeToExtras() const
{
    QMap<QString, QString> result;

    const int targetIdx = ui->targetComboBox->currentIndex();
    if (targetIdx >= 0 && targetIdx < targetTypeOrder.size())
    {
        result[kType] = targetTypeOrder.at(targetIdx);
    }

    const int queryIdx = ui->queryComboBox->currentIndex();
    if (queryIdx >= 0 && queryIdx < subtypeOrder.size())
    {
        result[kSubType] = subtypeOrder.at(queryIdx);
    }

    result[kCode] = ui->productLineEdit->text();

    switch (queryIdx)
    {
    case 0: // EXISTS
        break;
    case 1: // VERSION
        result[kMin] = ui->minLineEdit->text();
        result[kMax] = ui->maxLineEdit->text();
        result[kGte] = (ui->minComboBox->currentIndex() == 0)
                            ? QStringLiteral("1") : QStringLiteral("0");
        result[kLte] = (ui->maxComboBox->currentIndex() == 1)
                            ? QStringLiteral("1") : QStringLiteral("0");
        break;
    case 2: // GET_PROPERTY
        result[kItem] = ui->propertyNameLineEdit->text();
        result[kValue] = ui->variableNameLineEdit->text();
        break;
    case 3: // MATCH_PROPERTY
        result[kItem] = ui->propertyNameLineEdit->text();
        result[kValue] = ui->propertyValueLineEdit->text();
        break;
    case 4: // GET_INFORMATION
        result[kItem] = ui->infoItemComboBox->currentText();
        result[kValue] = ui->variableNameLineEdit->text();
        break;
    case 5: // MATCH_INFORMATION
        result[kItem] = ui->infoItemComboBox->currentText();
        result[kValue] = ui->infoValueLineEdit->text();
        break;
    }

    return result;
}

} // namespace preferences
