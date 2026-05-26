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

#include "registrymatchwidget.h"
#include "ui_registrymatchwidget.h"

namespace preferences
{

namespace
{
const QString kType         = QStringLiteral("type");
const QString kSubType      = QStringLiteral("subtype");
const QString kHive         = QStringLiteral("hive");
const QString kKey          = QStringLiteral("key");
const QString kValueName    = QStringLiteral("valueName");
const QString kValueType    = QStringLiteral("valueType");
const QString kValueData    = QStringLiteral("valueData");
const QString kVariableName = QStringLiteral("variableName");
const QString kMin          = QStringLiteral("min");
const QString kMax          = QStringLiteral("max");
const QString kGte          = QStringLiteral("gte");
const QString kLte          = QStringLiteral("lte");
const QString kVersion      = QStringLiteral("version");

const QStringList typeOrder = {
    QStringLiteral("KEYEXISTS"),
    QStringLiteral("VALUEEXISTS"),
    QStringLiteral("MATCHVALUE"),
    QStringLiteral("GETVALUE"),
};

const QStringList subtypeOrder = {
    QStringLiteral("EQUALHEX"),
    QStringLiteral("EQUALDEC"),
    QStringLiteral("SUBSTRING"),
    QStringLiteral("VERSION"),
};

const QStringList hiveOrder = {
    QStringLiteral("HKEY_CLASSES_ROOT"),
    QStringLiteral("HKEY_CURRENT_USER"),
    QStringLiteral("HKEY_LOCAL_MACHINE"),
    QStringLiteral("HKEY_USERS"),
    QStringLiteral("HKEY_CURRENT_CONFIG"),
};

const QStringList valueTypeOrder = {
    QString(),
    QStringLiteral("REG_SZ"),
    QStringLiteral("REG_EXPAND_SZ"),
    QStringLiteral("REG_MULTI_SZ"),
    QStringLiteral("REG_DWORD"),
    QStringLiteral("REG_BINARY"),
};

void selectComboText(QComboBox *combo, const QString &value)
{
    const int idx = combo->findText(value);
    if (idx >= 0)
    {
        combo->setCurrentIndex(idx);
    }
}

void selectComboData(QComboBox *combo, const QStringList &order, const QString &value)
{
    const int idx = order.indexOf(value);
    if (idx >= 0 && idx < combo->count())
    {
        combo->setCurrentIndex(idx);
    }
}
} // namespace

RegistryMatchFilterWidget::RegistryMatchFilterWidget(QWidget *parent)
    : TargetingFilterWidget(parent)
    , ui(new Ui::RegistryMatchWidget)
    , m_version()
{
    ui->setupUi(this);

    ui->pathToolButton_1->setEnabled(false);
    ui->pathToolButton_1->setToolTip(tr("Selection dialog is not implemented yet"));
    ui->pathToolButton_2->setEnabled(false);
    ui->pathToolButton_2->setToolTip(tr("Selection dialog is not implemented yet"));
    ui->pathToolButton_3->setEnabled(false);
    ui->pathToolButton_3->setToolTip(tr("Selection dialog is not implemented yet"));
    ui->pathToolButton_4->setEnabled(false);
    ui->pathToolButton_4->setToolTip(tr("Selection dialog is not implemented yet"));

    connect(ui->matchComboBox_1, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &RegistryMatchFilterWidget::onMatchTypeChanged);
    connect(ui->matchComboBox_2, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &RegistryMatchFilterWidget::onMatchTypeChanged);
    connect(ui->matchComboBox_3, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &RegistryMatchFilterWidget::onMatchTypeChanged);
    connect(ui->matchComboBox_4, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &RegistryMatchFilterWidget::onMatchTypeChanged);
    connect(ui->dataMatchComboBox_3, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &RegistryMatchFilterWidget::onDataMatchChanged);

    ui->stackedWidget->setCurrentIndex(0);
    onMatchTypeChanged(0);
    onDataMatchChanged(0);
}

RegistryMatchFilterWidget::~RegistryMatchFilterWidget()
{
    delete ui;
}

QStringList RegistryMatchFilterWidget::knownKeys() const
{
    return {kType, kSubType, kHive, kKey, kValueName, kValueType, kValueData,
            kVariableName, kMin, kMax, kGte, kLte, kVersion};
}

void RegistryMatchFilterWidget::onMatchTypeChanged(int matchIndex)
{
    const int page = matchIndex;
    ui->stackedWidget->setCurrentIndex(page);

    ui->matchComboBox_1->setCurrentIndex(matchIndex);
    ui->matchComboBox_2->setCurrentIndex(matchIndex);
    ui->matchComboBox_3->setCurrentIndex(matchIndex);
    ui->matchComboBox_4->setCurrentIndex(matchIndex);
}

void RegistryMatchFilterWidget::onDataMatchChanged(int index)
{
    ui->valDataLineEdit_3->setVisible(true);
    ui->valDataLabel_3->setVisible(true);
    ui->regMinComboBox->setVisible(false);
    ui->regMinLineEdit->setVisible(false);
    ui->regMaxComboBox->setVisible(false);
    ui->regMaxLineEdit->setVisible(false);

    if (index == 0) // Any
    {
        ui->valDataLabel_3->setText(tr("Value data:"));
    }
    else if (index == 1) // Substring match
    {
        ui->valDataLabel_3->setText(tr("Substring:"));
    }
    else if (index == 2) // Version match
    {
        ui->valDataLabel_3->setText(tr("Version:"));
        ui->valDataLineEdit_3->setVisible(false);
        ui->regMinComboBox->setVisible(true);
        ui->regMinLineEdit->setVisible(true);
        ui->regMaxComboBox->setVisible(true);
        ui->regMaxLineEdit->setVisible(true);
    }
}

void RegistryMatchFilterWidget::readFromExtras(const QMap<QString, QString> &extras)
{
    const QString type = extras.value(kType);
    const int typeIdx  = typeOrder.indexOf(type);

    const QString hive = extras.value(kHive);
    const int hiveIdx  = hiveOrder.indexOf(hive);

    const QString vtype = extras.value(kValueType);
    const int vtypeIdx  = valueTypeOrder.indexOf(vtype);

    const QString sub = extras.value(kSubType);
    const int subIdx  = subtypeOrder.indexOf(sub);
    m_version = extras.value(kVersion);

    switch (typeIdx)
    {
    case 0: // KEYEXISTS
    {
        ui->stackedWidget->setCurrentIndex(0);
        ui->matchComboBox_1->setCurrentIndex(0);
        if (hiveIdx >= 0) ui->hiveComboBox_1->setCurrentIndex(hiveIdx);
        ui->pathLineEdit_1->setText(extras.value(kKey));
        break;
    }
    case 1: // VALUEEXISTS
    {
        ui->stackedWidget->setCurrentIndex(1);
        ui->matchComboBox_2->setCurrentIndex(1);
        if (hiveIdx >= 0) ui->hiveComboBox_2->setCurrentIndex(hiveIdx);
        ui->pathLineEdit_2->setText(extras.value(kKey));
        ui->valNameLineEdit_2->setText(extras.value(kValueName));
        if (vtypeIdx >= 0) ui->valTypeComboBox_2->setCurrentIndex(vtypeIdx);
        break;
    }
    case 2: // MATCHVALUE
    {
        ui->stackedWidget->setCurrentIndex(2);
        ui->matchComboBox_3->setCurrentIndex(2);
        if (hiveIdx >= 0) ui->hiveComboBox_3->setCurrentIndex(hiveIdx);
        ui->pathLineEdit_3->setText(extras.value(kKey));
        ui->valNameLineEdit_3->setText(extras.value(kValueName));
        if (vtypeIdx >= 0) ui->valTypeComboBox_3->setCurrentIndex(vtypeIdx);
        if (subIdx >= 0) ui->dataMatchComboBox_3->setCurrentIndex(subIdx);

        if (subIdx == 3) // VERSION
        {
            ui->regMinLineEdit->setText(extras.value(kMin, QStringLiteral("0.0.0.0")));
            ui->regMaxLineEdit->setText(extras.value(kMax, QStringLiteral("0.0.0.0")));
            const bool gte = extras.value(kGte, QStringLiteral("0")) != QLatin1String("0");
            ui->regMinComboBox->setCurrentIndex(gte ? 0 : 1);
            const bool lte = extras.value(kLte, QStringLiteral("0")) != QLatin1String("0");
            ui->regMaxComboBox->setCurrentIndex(lte ? 1 : 0);
        }
        else
        {
            ui->valDataLineEdit_3->setText(extras.value(kValueData));
        }
        break;
    }
    case 3: // GETVALUE
    {
        ui->stackedWidget->setCurrentIndex(3);
        ui->matchComboBox_4->setCurrentIndex(3);
        if (hiveIdx >= 0) ui->hiveComboBox_4->setCurrentIndex(hiveIdx);
        ui->pathLineEdit_4->setText(extras.value(kKey));
        ui->valNameLineEdit_4->setText(extras.value(kValueName));
        if (vtypeIdx >= 0) ui->valTypeComboBox_4->setCurrentIndex(vtypeIdx);
        ui->varNameLineEdit_4->setText(extras.value(kVariableName));
        break;
    }
    default:
        ui->stackedWidget->setCurrentIndex(0);
        if (hiveIdx >= 0) ui->hiveComboBox_1->setCurrentIndex(hiveIdx);
        ui->pathLineEdit_1->setText(extras.value(kKey));
        break;
    }
}

QMap<QString, QString> RegistryMatchFilterWidget::writeToExtras() const
{
    QMap<QString, QString> result;

    const int page = ui->stackedWidget->currentIndex();

    result[kType] = (page >= 0 && page < typeOrder.size()) ? typeOrder.at(page)
                                                            : QStringLiteral("KEYEXISTS");

    switch (page)
    {
    case 0: // KEYEXISTS
    {
        result[kHive] = ui->hiveComboBox_1->currentText();
        result[kKey]  = ui->pathLineEdit_1->text();
        break;
    }
    case 1: // VALUEEXISTS
    {
        result[kHive]      = ui->hiveComboBox_2->currentText();
        result[kKey]       = ui->pathLineEdit_2->text();
        result[kValueName] = ui->valNameLineEdit_2->text();
        const int vt = ui->valTypeComboBox_2->currentIndex();
        if (vt >= 0 && vt < valueTypeOrder.size() && !valueTypeOrder.at(vt).isEmpty())
        {
            result[kValueType] = valueTypeOrder.at(vt);
        }
        break;
    }
    case 2: // MATCHVALUE
    {
        result[kHive]      = ui->hiveComboBox_3->currentText();
        result[kKey]       = ui->pathLineEdit_3->text();
        result[kValueName] = ui->valNameLineEdit_3->text();
        const int vt = ui->valTypeComboBox_3->currentIndex();
        if (vt >= 0 && vt < valueTypeOrder.size() && !valueTypeOrder.at(vt).isEmpty())
        {
            result[kValueType] = valueTypeOrder.at(vt);
        }
        const int st = ui->dataMatchComboBox_3->currentIndex();
        if (st >= 0 && st < subtypeOrder.size())
        {
            result[kSubType] = subtypeOrder.at(st);
        }

        if (st == 3) // VERSION
        {
            result[kMin] = ui->regMinLineEdit->text();
            result[kMax] = ui->regMaxLineEdit->text();
            result[kGte] = (ui->regMinComboBox->currentIndex() == 0)
                                ? QStringLiteral("1") : QStringLiteral("0");
            result[kLte] = (ui->regMaxComboBox->currentIndex() == 1)
                                ? QStringLiteral("1") : QStringLiteral("0");
        }
        else
        {
            result[kValueData] = ui->valDataLineEdit_3->text();
        }
        break;
    }
    case 3: // GETVALUE
    {
        result[kHive]      = ui->hiveComboBox_4->currentText();
        result[kKey]       = ui->pathLineEdit_4->text();
        result[kValueName] = ui->valNameLineEdit_4->text();
        const int vt = ui->valTypeComboBox_4->currentIndex();
        if (vt >= 0 && vt < valueTypeOrder.size() && !valueTypeOrder.at(vt).isEmpty())
        {
            result[kValueType] = valueTypeOrder.at(vt);
        }
        result[kVariableName] = ui->varNameLineEdit_4->text();
        break;
    }
    }

    if (!m_version.isEmpty())
    {
        result[kVersion] = m_version;
    }

    return result;
}

} // namespace preferences
