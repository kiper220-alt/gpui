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

#include "languagewriter.h"

#include "languageschema.h"

namespace preferences
{

std::unique_ptr<::FilterLanguage> LanguageWriter::write(
    const QMap<QString, QString> &extras,
    const QString &boolValue,
    const QString &notValue)
{
    auto boolType = ::EnumFilterBool(boolValue.toStdString());
    auto notType  = ::xml_schema::String(notValue.toStdString());
    auto langVal  = static_cast<unsigned char>(extras.value(QStringLiteral("language")).toUInt());
    auto locVal   = static_cast<unsigned char>(extras.value(QStringLiteral("locale"), QStringLiteral("0")).toUInt());

    auto result = std::make_unique<::FilterLanguage>(boolType, notType, langVal, locVal);

    const QString defaultVal = extras.value(QStringLiteral("default"));
    if (!defaultVal.isEmpty())
    {
        result->default_(::EnumTrueFalse(defaultVal.toStdString()));
    }

    const QString systemVal = extras.value(QStringLiteral("system"));
    if (!systemVal.isEmpty())
    {
        result->system(::EnumTrueFalse(systemVal.toStdString()));
    }

    const QString nativeVal = extras.value(QStringLiteral("native"));
    if (!nativeVal.isEmpty())
    {
        result->native(::EnumTrueFalse(nativeVal.toStdString()));
    }

    const QString displayName = extras.value(QStringLiteral("displayName"));
    if (!displayName.isEmpty())
    {
        result->displayName(::xml_schema::String(displayName.toStdString()));
    }

    return result;
}

} // namespace preferences
