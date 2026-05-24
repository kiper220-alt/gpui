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

#include "securitygroupreader.h"

#include "securitygroupschema.h"

namespace preferences
{

QStringList SecurityGroupReader::knownKeys()
{
    return {QStringLiteral("name"), QStringLiteral("sid"),
            QStringLiteral("userContext"), QStringLiteral("primaryGroup"),
            QStringLiteral("localGroup")};
}

QMap<QString, QString> SecurityGroupReader::read(const ::FilterGroup &filter)
{
    QMap<QString, QString> result;

    result[QStringLiteral("name")] = QString::fromStdString(filter.name());
    result[QStringLiteral("sid")] = QString::fromStdString(filter.sid());
    result[QStringLiteral("userContext")] = QString::fromStdString(
        static_cast<const ::xml_schema::String &>(filter.userContext()));
    result[QStringLiteral("primaryGroup")] = QString::fromStdString(
        static_cast<const ::xml_schema::String &>(filter.primaryGroup()));
    result[QStringLiteral("localGroup")] = QString::fromStdString(
        static_cast<const ::xml_schema::String &>(filter.localGroup()));

    return result;
}

} // namespace preferences
