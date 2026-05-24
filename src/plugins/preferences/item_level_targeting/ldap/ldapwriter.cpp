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

#include "ldapwriter.h"

#include "ldapschema.h"

namespace preferences
{

std::unique_ptr<::FilterLdap> LdapWriter::write(
    const QMap<QString, QString> &extras,
    const QString &boolValue,
    const QString &notValue)
{
    auto boolType   = ::EnumFilterBool(boolValue.toStdString());
    auto notType    = ::xml_schema::String(notValue.toStdString());
    auto bindingVal = ::xml_schema::String(extras.value(QStringLiteral("binding")).toStdString());

    auto result = std::make_unique<::FilterLdap>(boolType, notType, bindingVal);

    const QString searchFilter = extras.value(QStringLiteral("searchFilter"));
    if (!searchFilter.isEmpty())
    {
        result->searchFilter(::xml_schema::String(searchFilter.toStdString()));
    }

    const QString variableName = extras.value(QStringLiteral("variableName"));
    if (!variableName.isEmpty())
    {
        result->variableName(::xml_schema::String(variableName.toStdString()));
    }

    const QString attribute = extras.value(QStringLiteral("attribute"));
    if (!attribute.isEmpty())
    {
        result->attribute(::xml_schema::String(attribute.toStdString()));
    }

    return result;
}

} // namespace preferences
