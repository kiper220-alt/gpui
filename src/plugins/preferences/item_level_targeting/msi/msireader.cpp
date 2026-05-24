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

#include "msireader.h"

#include "msischema.h"

namespace preferences
{

QStringList MsiReader::knownKeys()
{
    return {QStringLiteral("type"), QStringLiteral("subtype"), QStringLiteral("code"),
            QStringLiteral("item"), QStringLiteral("value"),  QStringLiteral("min"),
            QStringLiteral("max"), QStringLiteral("gte"),   QStringLiteral("lte")};
}

QMap<QString, QString> MsiReader::read(const ::FilterMsi &filter)
{
    QMap<QString, QString> result;

    result[QStringLiteral("type")] = QString::fromStdString(
        static_cast<const ::xml_schema::String &>(filter.type()));
    result[QStringLiteral("subtype")] = QString::fromStdString(
        static_cast<const ::xml_schema::String &>(filter.subtype()));

    if (filter.code().present())
    {
        result[QStringLiteral("code")] = QString::fromStdString(*filter.code());
    }
    if (filter.item().present())
    {
        result[QStringLiteral("item")] = QString::fromStdString(*filter.item());
    }
    if (filter.value().present())
    {
        result[QStringLiteral("value")] = QString::fromStdString(*filter.value());
    }
    if (filter.min().present())
    {
        result[QStringLiteral("min")] = QString::fromStdString(*filter.min());
    }
    if (filter.max().present())
    {
        result[QStringLiteral("max")] = QString::fromStdString(*filter.max());
    }
    result[QStringLiteral("gte")] = QString::fromStdString(
        static_cast<const ::xml_schema::String &>(filter.gte()));
    result[QStringLiteral("lte")] = QString::fromStdString(
        static_cast<const ::xml_schema::String &>(filter.lte()));

    return result;
}

} // namespace preferences
