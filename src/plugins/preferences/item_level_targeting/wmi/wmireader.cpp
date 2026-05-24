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

#include "wmireader.h"

#include "wmischema.h"

namespace preferences
{

QStringList WmiReader::knownKeys()
{
    return {QStringLiteral("query"), QStringLiteral("nameSpace"),
            QStringLiteral("property"), QStringLiteral("variableName")};
}

QMap<QString, QString> WmiReader::read(const ::FilterWmi &filter)
{
    QMap<QString, QString> result;

    result[QStringLiteral("query")] = QString::fromStdString(filter.query());
    result[QStringLiteral("nameSpace")] = QString::fromStdString(filter.nameSpace());

    if (filter.property().present())
    {
        result[QStringLiteral("property")] = QString::fromStdString(*filter.property());
    }

    if (filter.variableName().present())
    {
        result[QStringLiteral("variableName")] = QString::fromStdString(*filter.variableName());
    }

    return result;
}

} // namespace preferences
