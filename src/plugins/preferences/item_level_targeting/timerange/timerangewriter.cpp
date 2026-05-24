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

#include "timerangewriter.h"

#include "timerangeschema.h"

#include <chrono>

namespace preferences
{

namespace
{

QStringList timeParts(const QString &value)
{
    QStringList parts = value.split(QLatin1Char(':'));
    if (parts.size() == 2)
    {
        parts.append(QStringLiteral("0"));
    }
    return parts;
}

} // namespace

std::unique_ptr<::FilterTime> TimeRangeWriter::write(
    const QMap<QString, QString> &extras,
    const QString &boolValue,
    const QString &notValue)
{
    auto boolType = ::EnumFilterBool(boolValue.toStdString());
    auto notType  = ::xml_schema::String(notValue.toStdString());

    const QStringList beginParts = timeParts(extras.value(QStringLiteral("begin")));
    const QStringList endParts   = timeParts(extras.value(QStringLiteral("end")));

    auto beginVal = ::xml_schema::Time(
        static_cast<unsigned short>(beginParts.value(0).toUShort()),
        static_cast<unsigned short>(beginParts.value(1).toUShort()),
        beginParts.value(2).toDouble());

    auto endVal = ::xml_schema::Time(
        static_cast<unsigned short>(endParts.value(0).toUShort()),
        static_cast<unsigned short>(endParts.value(1).toUShort()),
        endParts.value(2).toDouble());

    return std::make_unique<::FilterTime>(boolType, notType, beginVal, endVal);
}

} // namespace preferences
