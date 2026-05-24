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

#include "ldapreader.h"

#include "ldapschema.h"

namespace preferences
{

QStringList LdapReader::knownKeys()
{
    return {QStringLiteral("searchFilter"), QStringLiteral("binding"),
            QStringLiteral("variableName"), QStringLiteral("attribute")};
}

QMap<QString, QString> LdapReader::read(const ::FilterLdap &filter)
{
    QMap<QString, QString> result;

    result[QStringLiteral("binding")] = QString::fromStdString(filter.binding());

    if (filter.searchFilter().present())
    {
        result[QStringLiteral("searchFilter")] = QString::fromStdString(*filter.searchFilter());
    }

    if (filter.variableName().present())
    {
        result[QStringLiteral("variableName")] = QString::fromStdString(*filter.variableName());
    }

    if (filter.attribute().present())
    {
        result[QStringLiteral("attribute")] = QString::fromStdString(*filter.attribute());
    }

    return result;
}

} // namespace preferences
