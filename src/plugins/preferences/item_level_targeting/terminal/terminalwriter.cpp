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

#include "terminalwriter.h"

#include "terminalschema.h"

namespace preferences
{

std::unique_ptr<::FilterTerminal> TerminalWriter::write(
    const QMap<QString, QString> &extras,
    const QString &boolValue,
    const QString &notValue)
{
    auto boolType  = ::EnumFilterBool(boolValue.toStdString());
    auto notType   = ::xml_schema::String(notValue.toStdString());
    auto typeVal   = ::EnumFilterTerminalType(extras.value(QStringLiteral("type")).toStdString());
    auto optionVal = ::EnumFilterTerminalOption(extras.value(QStringLiteral("option")).toStdString());

    auto result = std::make_unique<::FilterTerminal>(boolType, notType, typeVal, optionVal);

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

    return result;
}

} // namespace preferences
