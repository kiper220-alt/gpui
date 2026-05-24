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

#include "registrywriter.h"

#include "registryschema.h"

namespace preferences
{

std::unique_ptr<::FilterRegistry> RegistryWriter::write(
    const QMap<QString, QString> &extras,
    const QString &boolValue,
    const QString &notValue)
{
    auto boolType = ::EnumFilterBool(boolValue.toStdString());
    auto notType  = ::xml_schema::String(notValue.toStdString());
    auto keyVal   = ::xml_schema::String(extras.value(QStringLiteral("key")).toStdString());

    auto result = std::make_unique<::FilterRegistry>(boolType, notType, keyVal);

    const QString type = extras.value(QStringLiteral("type"));
    if (!type.isEmpty())
    {
        result->type(::EnumFilterRegistryType(type.toStdString()));
    }
    const QString subtype = extras.value(QStringLiteral("subtype"));
    if (!subtype.isEmpty())
    {
        result->subtype(::EnumFilterRegistrySubType(subtype.toStdString()));
    }
    const QString hive = extras.value(QStringLiteral("hive"));
    if (!hive.isEmpty())
    {
        result->hive(::EnumFilterRegistryHive(hive.toStdString()));
    }
    const QString gte = extras.value(QStringLiteral("gte"));
    if (!gte.isEmpty())
    {
        result->gte(::EnumTrueFalse(gte.toStdString()));
    }
    const QString lte = extras.value(QStringLiteral("lte"));
    if (!lte.isEmpty())
    {
        result->lte(::EnumTrueFalse(lte.toStdString()));
    }
    const QString valueName = extras.value(QStringLiteral("valueName"));
    if (!valueName.isEmpty())
    {
        result->valueName(::xml_schema::String(valueName.toStdString()));
    }
    const QString valueType = extras.value(QStringLiteral("valueType"));
    if (!valueType.isEmpty())
    {
        result->valueType(::EnumFilterRegistryValueType(valueType.toStdString()));
    }
    const QString valueData = extras.value(QStringLiteral("valueData"));
    if (!valueData.isEmpty())
    {
        result->valueData(::xml_schema::String(valueData.toStdString()));
    }
    const QString variableName = extras.value(QStringLiteral("variableName"));
    if (!variableName.isEmpty())
    {
        result->variableName(::xml_schema::String(variableName.toStdString()));
    }
    const QString min = extras.value(QStringLiteral("min"));
    if (!min.isEmpty())
    {
        result->min(::xml_schema::String(min.toStdString()));
    }
    const QString max = extras.value(QStringLiteral("max"));
    if (!max.isEmpty())
    {
        result->max(::xml_schema::String(max.toStdString()));
    }
    const QString version = extras.value(QStringLiteral("version"));
    if (!version.isEmpty())
    {
        result->version(::xml_schema::String(version.toStdString()));
    }

    return result;
}

} // namespace preferences
