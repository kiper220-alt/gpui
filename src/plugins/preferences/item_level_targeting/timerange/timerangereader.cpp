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

#include "timerangereader.h"

#include "timerangeschema.h"

namespace preferences
{

namespace
{

QString formatTime(const ::xml_schema::Time &time)
{
    if (static_cast<int>(time.seconds()) == 0)
    {
        return QStringLiteral("%1:%2")
            .arg(time.hours(), 2, 10, QLatin1Char('0'))
            .arg(time.minutes(), 2, 10, QLatin1Char('0'));
    }

    return QStringLiteral("%1:%2:%3")
        .arg(time.hours(), 2, 10, QLatin1Char('0'))
        .arg(time.minutes(), 2, 10, QLatin1Char('0'))
        .arg(static_cast<int>(time.seconds()), 2, 10, QLatin1Char('0'));
}

} // namespace

QStringList TimeRangeReader::knownKeys()
{
    return {QStringLiteral("begin"), QStringLiteral("end")};
}

QMap<QString, QString> TimeRangeReader::read(const ::FilterTime &filter)
{
    QMap<QString, QString> result;

    result[QStringLiteral("begin")] = formatTime(filter.begin());

    result[QStringLiteral("end")] = formatTime(filter.end());

    return result;
}

} // namespace preferences
