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

#include "terminalreader.h"

#include "terminalschema.h"

namespace preferences
{

QStringList TerminalReader::knownKeys()
{
    return {QStringLiteral("type"), QStringLiteral("option"), QStringLiteral("value"),
            QStringLiteral("min"), QStringLiteral("max")};
}

QMap<QString, QString> TerminalReader::read(const ::FilterTerminal &filter)
{
    QMap<QString, QString> result;

    result[QStringLiteral("type")] = QString::fromStdString(
        static_cast<const ::xml_schema::String &>(filter.type()));

    result[QStringLiteral("option")] = QString::fromStdString(
        static_cast<const ::xml_schema::String &>(filter.option()));

    result[QStringLiteral("value")] = QString::fromStdString(filter.value());
    result[QStringLiteral("min")] = QString::fromStdString(filter.min());
    result[QStringLiteral("max")] = QString::fromStdString(filter.max());

    return result;
}

} // namespace preferences
