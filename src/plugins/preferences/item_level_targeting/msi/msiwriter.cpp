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

#include "msiwriter.h"

#include "msischema.h"

namespace preferences
{

std::unique_ptr<::FilterMsi> MsiWriter::write(
    const QMap<QString, QString> &extras,
    const QString &boolValue,
    const QString &notValue)
{
    auto boolType  = ::EnumFilterBool(boolValue.toStdString());
    auto notType   = ::xml_schema::String(notValue.toStdString());
    auto typeVal   = ::EnumFilterMsiType(extras.value(QStringLiteral("type")).toStdString());
    auto subTypeVal = ::EnumFilterMsiSubType(extras.value(QStringLiteral("subtype")).toStdString());

    auto result = std::make_unique<::FilterMsi>(boolType, notType, typeVal, subTypeVal);

    const QString code = extras.value(QStringLiteral("code"));
    if (!code.isEmpty())
    {
        result->code(::xml_schema::String(code.toStdString()));
    }
    const QString item = extras.value(QStringLiteral("item"));
    if (!item.isEmpty())
    {
        result->item(::xml_schema::String(item.toStdString()));
    }
    const QString value = extras.value(QStringLiteral("value"));
    if (!value.isEmpty())
    {
        result->value(::xml_schema::String(value.toStdString()));
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

    return result;
}

} // namespace preferences
