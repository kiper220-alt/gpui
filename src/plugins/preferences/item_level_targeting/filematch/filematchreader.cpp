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

#include "filematchreader.h"

#include "filematchschema.h"

namespace preferences
{

QStringList FileMatchReader::knownKeys()
{
    return {QStringLiteral("type"), QStringLiteral("path"), QStringLiteral("folder"),
            QStringLiteral("min"),  QStringLiteral("max"), QStringLiteral("gte"),
            QStringLiteral("lte")};
}

QMap<QString, QString> FileMatchReader::read(const ::FilterFile &filter)
{
    QMap<QString, QString> result;

    result[QStringLiteral("path")] = QString::fromStdString(filter.path());

    result[QStringLiteral("type")] = QString::fromStdString(
        static_cast<const ::xml_schema::String &>(filter.type()));

    result[QStringLiteral("folder")] = QString::fromStdString(
        static_cast<const ::xml_schema::String &>(filter.folder()));

    result[QStringLiteral("gte")] = QString::fromStdString(
        static_cast<const ::xml_schema::String &>(filter.gte()));

    result[QStringLiteral("lte")] = QString::fromStdString(
        static_cast<const ::xml_schema::String &>(filter.lte()));

    if (filter.min().present())
    {
        result[QStringLiteral("min")] = QString::fromStdString(*filter.min());
    }

    if (filter.max().present())
    {
        result[QStringLiteral("max")] = QString::fromStdString(*filter.max());
    }

    return result;
}

} // namespace preferences
