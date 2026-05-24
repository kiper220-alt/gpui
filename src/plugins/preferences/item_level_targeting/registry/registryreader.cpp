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

#include "registryreader.h"

#include "registryschema.h"

namespace preferences
{

QStringList RegistryReader::knownKeys()
{
    return {QStringLiteral("type"), QStringLiteral("subtype"), QStringLiteral("hive"),
            QStringLiteral("key"), QStringLiteral("valueName"), QStringLiteral("valueType"),
            QStringLiteral("valueData"), QStringLiteral("variableName"),
            QStringLiteral("min"), QStringLiteral("max"),
            QStringLiteral("gte"), QStringLiteral("lte"),
            QStringLiteral("version")};
}

QMap<QString, QString> RegistryReader::read(const ::FilterRegistry &filter)
{
    QMap<QString, QString> result;

    result[QStringLiteral("key")] = QString::fromStdString(filter.key());
    result[QStringLiteral("type")] = QString::fromStdString(
        static_cast<const ::xml_schema::String &>(filter.type()));
    result[QStringLiteral("subtype")] = QString::fromStdString(
        static_cast<const ::xml_schema::String &>(filter.subtype()));
    result[QStringLiteral("hive")] = QString::fromStdString(
        static_cast<const ::xml_schema::String &>(filter.hive()));
    result[QStringLiteral("gte")] = QString::fromStdString(
        static_cast<const ::xml_schema::String &>(filter.gte()));
    result[QStringLiteral("lte")] = QString::fromStdString(
        static_cast<const ::xml_schema::String &>(filter.lte()));

    if (filter.valueName().present())
    {
        result[QStringLiteral("valueName")] = QString::fromStdString(*filter.valueName());
    }
    result[QStringLiteral("valueType")] = QString::fromStdString(
        static_cast<const ::xml_schema::String &>(filter.valueType()));
    if (filter.valueData().present())
    {
        result[QStringLiteral("valueData")] = QString::fromStdString(*filter.valueData());
    }
    if (filter.variableName().present())
    {
        result[QStringLiteral("variableName")] = QString::fromStdString(*filter.variableName());
    }
    if (filter.min().present())
    {
        result[QStringLiteral("min")] = QString::fromStdString(*filter.min());
    }
    if (filter.max().present())
    {
        result[QStringLiteral("max")] = QString::fromStdString(*filter.max());
    }
    if (filter.version().present())
    {
        result[QStringLiteral("version")] = QString::fromStdString(*filter.version());
    }

    return result;
}

} // namespace preferences
