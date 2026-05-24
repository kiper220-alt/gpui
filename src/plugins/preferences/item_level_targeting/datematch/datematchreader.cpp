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

#include "datematchreader.h"

#include "datematchschema.h"

namespace preferences
{

QStringList DateMatchReader::knownKeys()
{
    return {QStringLiteral("period"), QStringLiteral("dow"),
            QStringLiteral("day"),   QStringLiteral("month"),
            QStringLiteral("year")};
}

QMap<QString, QString> DateMatchReader::read(const ::FilterDate &filter)
{
    QMap<QString, QString> result;

    result[QStringLiteral("period")] = QString::fromStdString(
        static_cast<const ::xml_schema::String &>(filter.period()));

    if (filter.dow().present())
    {
        result[QStringLiteral("dow")] = QString::fromStdString(
            static_cast<const ::xml_schema::String &>(*filter.dow()));
    }

    if (filter.day().present())
    {
        result[QStringLiteral("day")] = QString::number(static_cast<unsigned>(*filter.day()));
    }

    if (filter.month().present())
    {
        result[QStringLiteral("month")] = QString::number(static_cast<unsigned>(*filter.month()));
    }

    if (filter.year().present())
    {
        result[QStringLiteral("year")] = QString::number(*filter.year());
    }

    return result;
}

} // namespace preferences
