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

#include "processingwriter.h"

#include "processingschema.h"

namespace preferences
{

namespace
{
void setIfPresent(::FilterProcMode &f, const QMap<QString, QString> &extras, const QString &key,
                  void (::FilterProcMode::*setter)(const ::EnumTrueFalse &))
{
    const QString val = extras.value(key);
    if (!val.isEmpty())
    {
        (f.*setter)(::EnumTrueFalse(val.toStdString()));
    }
}

} // namespace

std::unique_ptr<::FilterProcMode> ProcessingWriter::write(
    const QMap<QString, QString> &extras,
    const QString &boolValue,
    const QString &notValue)
{
    auto boolType = ::EnumFilterBool(boolValue.toStdString());
    auto notType  = ::xml_schema::String(notValue.toStdString());

    auto result = std::make_unique<::FilterProcMode>(boolType, notType);

    setIfPresent(*result, extras, QStringLiteral("syncFore"), &::FilterProcMode::syncFore);
    setIfPresent(*result, extras, QStringLiteral("asyncFore"), &::FilterProcMode::asyncFore);
    setIfPresent(*result, extras, QStringLiteral("backRefr"), &::FilterProcMode::backRefr);
    setIfPresent(*result, extras, QStringLiteral("forceRefr"), &::FilterProcMode::forceRefr);
    setIfPresent(*result, extras, QStringLiteral("linkTrns"), &::FilterProcMode::linkTrns);
    setIfPresent(*result, extras, QStringLiteral("noChg"), &::FilterProcMode::noChg);
    setIfPresent(*result, extras, QStringLiteral("rsopTrns"), &::FilterProcMode::rsopTrns);
    setIfPresent(*result, extras, QStringLiteral("safeBoot"), &::FilterProcMode::safeBoot);
    setIfPresent(*result, extras, QStringLiteral("slowLink"), &::FilterProcMode::slowLink);
    setIfPresent(*result, extras, QStringLiteral("verbLog"), &::FilterProcMode::verbLog);
    setIfPresent(*result, extras, QStringLiteral("rsopEnbl"), &::FilterProcMode::rsopEnbl);

    return result;
}

} // namespace preferences
