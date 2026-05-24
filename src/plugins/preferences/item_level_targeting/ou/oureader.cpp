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

#include "oureader.h"

#include "ouschema.h"

namespace preferences
{

QStringList OuReader::knownKeys()
{
    return {QStringLiteral("name"), QStringLiteral("userContext"), QStringLiteral("directMember")};
}

QMap<QString, QString> OuReader::read(const ::FilterOrgUnit &filter)
{
    QMap<QString, QString> result;
    result[QStringLiteral("name")] = QString::fromStdString(filter.name());
    result[QStringLiteral("userContext")] = QString::fromStdString(
        static_cast<const ::xml_schema::String &>(filter.userContext()));
    result[QStringLiteral("directMember")] = QString::fromStdString(
        static_cast<const ::xml_schema::String &>(filter.directMember()));
    return result;
}

} // namespace preferences
