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

#include "wmiwriter.h"

#include "wmischema.h"

namespace preferences
{

std::unique_ptr<::FilterWmi> WmiWriter::write(
    const QMap<QString, QString> &extras,
    const QString &boolValue,
    const QString &notValue)
{
    auto boolType = ::EnumFilterBool(boolValue.toStdString());
    auto notType  = ::xml_schema::String(notValue.toStdString());
    auto queryVal = ::xml_schema::String(extras.value(QStringLiteral("query")).toStdString());

    auto result = std::make_unique<::FilterWmi>(boolType, notType, queryVal);

    const QString nameSpace = extras.value(QStringLiteral("nameSpace"));
    if (!nameSpace.isEmpty())
    {
        result->nameSpace(::xml_schema::String(nameSpace.toStdString()));
    }

    const QString property = extras.value(QStringLiteral("property"));
    if (!property.isEmpty())
    {
        result->property(::xml_schema::String(property.toStdString()));
    }

    const QString variableName = extras.value(QStringLiteral("variableName"));
    if (!variableName.isEmpty())
    {
        result->variableName(::xml_schema::String(variableName.toStdString()));
    }

    return result;
}

} // namespace preferences
