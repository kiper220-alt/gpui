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

#include "portablereader.h"

#include "portableschema.h"

namespace preferences
{

QStringList PortableReader::knownKeys()
{
    return {QStringLiteral("unknown"), QStringLiteral("docked"), QStringLiteral("undocked")};
}

QMap<QString, QString> PortableReader::read(const ::FilterPortable &filter)
{
    QMap<QString, QString> result;
    result[QStringLiteral("unknown")]  = QString::fromStdString(
        static_cast<const ::xml_schema::String &>(filter.unknown()));
    result[QStringLiteral("docked")]   = QString::fromStdString(
        static_cast<const ::xml_schema::String &>(filter.docked()));
    result[QStringLiteral("undocked")] = QString::fromStdString(
        static_cast<const ::xml_schema::String &>(filter.undocked()));
    return result;
}

} // namespace preferences
