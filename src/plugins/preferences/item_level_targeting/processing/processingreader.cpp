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

#include "processingreader.h"

#include "processingschema.h"

namespace preferences
{

QStringList ProcessingReader::knownKeys()
{
    return {QStringLiteral("syncFore"),  QStringLiteral("asyncFore"),
            QStringLiteral("backRefr"),  QStringLiteral("forceRefr"),
            QStringLiteral("linkTrns"),  QStringLiteral("noChg"),
            QStringLiteral("rsopTrns"),  QStringLiteral("safeBoot"),
            QStringLiteral("slowLink"),  QStringLiteral("verbLog"),
            QStringLiteral("rsopEnbl")};
}

QMap<QString, QString> ProcessingReader::read(const ::FilterProcMode &filter)
{
    auto toStr = [](const ::EnumTrueFalse &v) -> QString {
        return QString::fromStdString(static_cast<const ::xml_schema::String &>(v));
    };

    return {
        {QStringLiteral("syncFore"), toStr(filter.syncFore())},
        {QStringLiteral("asyncFore"), toStr(filter.asyncFore())},
        {QStringLiteral("backRefr"), toStr(filter.backRefr())},
        {QStringLiteral("forceRefr"), toStr(filter.forceRefr())},
        {QStringLiteral("linkTrns"), toStr(filter.linkTrns())},
        {QStringLiteral("noChg"), toStr(filter.noChg())},
        {QStringLiteral("rsopTrns"), toStr(filter.rsopTrns())},
        {QStringLiteral("safeBoot"), toStr(filter.safeBoot())},
        {QStringLiteral("slowLink"), toStr(filter.slowLink())},
        {QStringLiteral("verbLog"), toStr(filter.verbLog())},
        {QStringLiteral("rsopEnbl"), toStr(filter.rsopEnbl())},
    };
}

} // namespace preferences
