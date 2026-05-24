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

#include "timerangefilterwidget.h"
#include "ui_timerangewidget.h"

#include <QTime>

namespace preferences
{

namespace
{
constexpr const char *kMinuteTimeFormat = "HH:mm";
constexpr const char *kSecondTimeFormat = "HH:mm:ss";

QTime parseTime(const QString &value)
{
    QTime time = QTime::fromString(value, kMinuteTimeFormat);
    if (time.isValid())
    {
        return time;
    }
    return QTime::fromString(value, kSecondTimeFormat);
}
}

TimeRangeFilterWidget::TimeRangeFilterWidget(QWidget *parent)
    : TargetingFilterWidget(parent)
    , ui(new Ui::TimeRangeWidget)
{
    ui->setupUi(this);
}

TimeRangeFilterWidget::~TimeRangeFilterWidget()
{
    delete ui;
}

QStringList TimeRangeFilterWidget::knownKeys() const
{
    return {QStringLiteral("begin"), QStringLiteral("end")};
}

void TimeRangeFilterWidget::readFromExtras(const QMap<QString, QString> &extras)
{
    const QTime begin = parseTime(extras.value(QStringLiteral("begin")));
    const QTime end   = parseTime(extras.value(QStringLiteral("end")));
    if (begin.isValid())
    {
        ui->minTimeEdit->setTime(begin);
    }
    if (end.isValid())
    {
        ui->maxTimeEdit->setTime(end);
    }
}

QMap<QString, QString> TimeRangeFilterWidget::writeToExtras() const
{
    return {
        {QStringLiteral("begin"), ui->minTimeEdit->time().toString(kMinuteTimeFormat)},
        {QStringLiteral("end"),   ui->maxTimeEdit->time().toString(kMinuteTimeFormat)},
    };
}

} // namespace preferences
