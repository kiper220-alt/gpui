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

#include "languagereader.h"

#include "languageschema.h"

namespace preferences
{

QStringList LanguageReader::knownKeys()
{
    return {QStringLiteral("default"), QStringLiteral("system"),
            QStringLiteral("native"), QStringLiteral("displayName"),
            QStringLiteral("language"), QStringLiteral("locale")};
}

QMap<QString, QString> LanguageReader::read(const ::FilterLanguage &filter)
{
    QMap<QString, QString> result;

    result[QStringLiteral("default")] = QString::fromStdString(
        static_cast<const ::xml_schema::String &>(filter.default_()));
    result[QStringLiteral("system")] = QString::fromStdString(
        static_cast<const ::xml_schema::String &>(filter.system()));
    result[QStringLiteral("native")] = QString::fromStdString(
        static_cast<const ::xml_schema::String &>(filter.native()));
    result[QStringLiteral("language")] = QString::number(static_cast<unsigned>(filter.language()));
    result[QStringLiteral("locale")] = QString::number(static_cast<unsigned>(filter.locale()));

    if (filter.displayName().present())
    {
        result[QStringLiteral("displayName")] = QString::fromStdString(*filter.displayName());
    }

    return result;
}

} // namespace preferences
