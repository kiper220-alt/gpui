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

#include "datematchwriter.h"

#include "datematchschema.h"

namespace preferences
{

std::unique_ptr<::FilterDate> DateMatchWriter::write(
    const QMap<QString, QString> &extras,
    const QString &boolValue,
    const QString &notValue)
{
    auto boolType  = ::EnumFilterBool(boolValue.toStdString());
    auto notType   = ::xml_schema::String(notValue.toStdString());
    auto periodVal = ::EnumFilterDatePeriod(extras.value(QStringLiteral("period")).toStdString());

    auto result = std::make_unique<::FilterDate>(boolType, notType, periodVal);

    const QString dow = extras.value(QStringLiteral("dow"));
    if (!dow.isEmpty())
    {
        result->dow(::EnumFilterDateDow(dow.toStdString()));
    }

    const QString day = extras.value(QStringLiteral("day"));
    if (!day.isEmpty())
    {
        result->day(static_cast<unsigned char>(day.toUInt()));
    }

    const QString month = extras.value(QStringLiteral("month"));
    if (!month.isEmpty())
    {
        result->month(static_cast<unsigned char>(month.toUInt()));
    }

    const QString year = extras.value(QStringLiteral("year"));
    if (!year.isEmpty())
    {
        result->year(static_cast<unsigned short>(year.toUInt()));
    }

    return result;
}

} // namespace preferences
