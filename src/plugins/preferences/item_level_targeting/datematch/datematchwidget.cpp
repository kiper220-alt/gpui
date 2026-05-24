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

#include "datematchwidget.h"
#include "ui_datematchwidget.h"

#include <QDate>

namespace preferences
{

namespace
{
const QString kPeriod = QStringLiteral("period");
const QString kDow    = QStringLiteral("dow");
const QString kDay    = QStringLiteral("day");
const QString kMonth  = QStringLiteral("month");
const QString kYear   = QStringLiteral("year");

const QStringList dowValues = {
    QStringLiteral("SUN"),
    QStringLiteral("MON"),
    QStringLiteral("TUE"),
    QStringLiteral("WED"),
    QStringLiteral("THU"),
    QStringLiteral("FRI"),
    QStringLiteral("SAT"),
};

const QStringList periodValues = {
    QStringLiteral("WEEKLY"),
    QStringLiteral("MONTHLY"),
    QStringLiteral("YEARLY"),
};
} // namespace

DateMatchFilterWidget::DateMatchFilterWidget(QWidget *parent)
    : TargetingFilterWidget(parent)
    , ui(new Ui::DateMatchWidget)
{
    ui->setupUi(this);

    connect(ui->periodicComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DateMatchFilterWidget::onPeriodChanged);
    connect(ui->periodicComboBox_2, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DateMatchFilterWidget::onPeriodChanged);
    connect(ui->periodicComboBox_3, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DateMatchFilterWidget::onPeriodChanged);
    connect(ui->checkBox, &QCheckBox::toggled,
            this, &DateMatchFilterWidget::onYearlyToggled);

    ui->dateEdit->setDate(QDate::currentDate());
    ui->stackedWidget->setCurrentIndex(2);
    onPeriodChanged(0);
}

DateMatchFilterWidget::~DateMatchFilterWidget()
{
    delete ui;
}

QStringList DateMatchFilterWidget::knownKeys() const
{
    return {kPeriod, kDow, kDay, kMonth, kYear, QStringLiteral("yearly")};
}

void DateMatchFilterWidget::onPeriodChanged(int comboIndex)
{
    const int pageIndex = (comboIndex == 0) ? 2   // Weekly  -> page_3
                       : (comboIndex == 1) ? 1   // Monthly -> page_2
                       : 0;                      // On date -> page
    ui->stackedWidget->setCurrentIndex(pageIndex);

    ui->periodicComboBox->setCurrentIndex(comboIndex);
    ui->periodicComboBox_2->setCurrentIndex(comboIndex);
    ui->periodicComboBox_3->setCurrentIndex(comboIndex);
}

void DateMatchFilterWidget::onYearlyToggled(bool checked)
{
    ui->dateEdit->setDisplayFormat(checked ? QStringLiteral("dd/MM")
                                          : QStringLiteral("dd/MM/yyyy"));
}

void DateMatchFilterWidget::readFromExtras(const QMap<QString, QString> &extras)
{
    const QString period = extras.value(kPeriod, QStringLiteral("WEEKLY"));

    if (period == QLatin1String("WEEKLY"))
    {
        ui->periodicComboBox_3->setCurrentIndex(0);
        const int dowIdx = dowValues.indexOf(extras.value(kDow));
        if (dowIdx >= 0)
        {
            ui->dayComboBox->setCurrentIndex(dowIdx);
        }
    }
    else if (period == QLatin1String("MONTHLY"))
    {
        ui->periodicComboBox_2->setCurrentIndex(1);
        const int day = extras.value(kDay, QStringLiteral("1")).toInt();
        ui->monthspinBox->setValue(qBound(1, day, 31));
    }
    else
    {
        ui->periodicComboBox->setCurrentIndex(2);
        ui->checkBox->setChecked(!extras.contains(kYear));

        const QDate today = QDate::currentDate();
        const int y = extras.value(kYear, QString::number(today.year())).toInt();
        const int m = extras.value(kMonth, QString::number(today.month())).toInt();
        const int d = extras.value(kDay, QString::number(today.day())).toInt();
        QDate date(qBound(1, y, 9999), qBound(1, m, 12), qBound(1, d, 31));
        if (!date.isValid())
        {
            date = QDate::currentDate();
        }
        ui->dateEdit->setDate(date);
    }
}

QMap<QString, QString> DateMatchFilterWidget::writeToExtras() const
{
    QMap<QString, QString> result;

    const int page = ui->stackedWidget->currentIndex();

    if (page == 2)
    {
        result[kPeriod] = QStringLiteral("WEEKLY");
        const int dowIdx = ui->dayComboBox->currentIndex();
        if (dowIdx >= 0 && dowIdx < dowValues.size())
        {
            result[kDow] = dowValues.at(dowIdx);
        }
    }
    else if (page == 1)
    {
        result[kPeriod] = QStringLiteral("MONTHLY");
        result[kDay] = QString::number(ui->monthspinBox->value());
    }
    else
    {
        result[kPeriod] = QStringLiteral("YEARLY");
        const QDate date = ui->dateEdit->date();
        result[kDay]   = QString::number(date.day());
        result[kMonth] = QString::number(date.month());
        if (!ui->checkBox->isChecked())
        {
            result[kYear] = QString::number(date.year());
        }
    }

    return result;
}

} // namespace preferences
