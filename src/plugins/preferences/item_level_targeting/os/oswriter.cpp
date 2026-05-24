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

#include "oswriter.h"

#include "osschema.h"

namespace preferences
{

std::unique_ptr<::FilterOs> OsWriter::write(
    const QMap<QString, QString> &extras,
    const QString &boolValue,
    const QString &notValue)
{
    auto boolType = ::EnumFilterBool(boolValue.toStdString());
    auto notType  = ::xml_schema::String(notValue.toStdString());

    auto result = std::make_unique<::FilterOs>(boolType, notType);

    const QString osClass = extras.value(QStringLiteral("class"));
    if (!osClass.isEmpty())
    {
        result->class_(::EnumFilterOsClass(osClass.toStdString()));
    }
    const QString version = extras.value(QStringLiteral("version"));
    if (!version.isEmpty())
    {
        result->version(::EnumFilterOsVersion(version.toStdString()));
    }
    const QString edition = extras.value(QStringLiteral("edition"));
    if (!edition.isEmpty())
    {
        result->edition(::EnumFilterOsEdition(edition.toStdString()));
    }
    const QString sp = extras.value(QStringLiteral("sp"));
    if (!sp.isEmpty())
    {
        result->sp(::EnumFilterOsSp(sp.toStdString()));
    }
    const QString type = extras.value(QStringLiteral("type"));
    if (!type.isEmpty())
    {
        result->type(::EnumFilterOsType(type.toStdString()));
    }

    return result;
}

} // namespace preferences
