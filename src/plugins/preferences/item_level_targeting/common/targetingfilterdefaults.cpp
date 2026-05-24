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

#include "targetingfilterdefaults.h"

#include <QDate>

namespace preferences
{

namespace
{

QString bool0()
{
    return QStringLiteral("0");
}

QString bool1()
{
    return QStringLiteral("1");
}

QMap<QString, QString> processingDefaults()
{
    return {
        {QStringLiteral("syncFore"), bool0()},
        {QStringLiteral("asyncFore"), bool0()},
        {QStringLiteral("backRefr"), bool0()},
        {QStringLiteral("forceRefr"), bool0()},
        {QStringLiteral("linkTrns"), bool0()},
        {QStringLiteral("noChg"), bool0()},
        {QStringLiteral("rsopTrns"), bool0()},
        {QStringLiteral("safeBoot"), bool0()},
        {QStringLiteral("slowLink"), bool0()},
        {QStringLiteral("verbLog"), bool0()},
        {QStringLiteral("rsopEnbl"), bool0()},
    };
}

void normalizeDateExtras(QMap<QString, QString> &extras)
{
    extras.remove(QStringLiteral("yearly"));

    if (extras.value(QStringLiteral("period")) != QLatin1String("YEARLY"))
    {
        return;
    }

    const QDate today = QDate::currentDate();
    if (extras.value(QStringLiteral("day")).isEmpty())
    {
        extras.insert(QStringLiteral("day"), QString::number(today.day()));
    }
    if (extras.value(QStringLiteral("month")).isEmpty())
    {
        extras.insert(QStringLiteral("month"), QString::number(today.month()));
    }
}

void normalizeProcessingExtras(QMap<QString, QString> &extras)
{
    extras.remove(QStringLiteral("synchFore"));
    extras.remove(QStringLiteral("asynchFore"));
}

} // namespace

QMap<QString, QString> defaultTargetingFilterExtras(const QString &filterName)
{
    if (filterName == QLatin1String("FilterBattery")
        || filterName == QLatin1String("FilterCollection")
        || filterName == QLatin1String("FilterPcmcia"))
    {
        return {};
    }
    if (filterName == QLatin1String("FilterComputer"))
    {
        return {{QStringLiteral("name"), QString()},
                {QStringLiteral("type"), QStringLiteral("NETBIOS")}};
    }
    if (filterName == QLatin1String("FilterCpu"))
    {
        return {{QStringLiteral("speedMHz"), QStringLiteral("500")}};
    }
    if (filterName == QLatin1String("FilterDate"))
    {
        return {{QStringLiteral("period"), QStringLiteral("WEEKLY")},
                {QStringLiteral("dow"), QStringLiteral("SUN")}};
    }
    if (filterName == QLatin1String("FilterDisk"))
    {
        return {{QStringLiteral("drive"), QStringLiteral("%SystemDrive%")},
                {QStringLiteral("freeSpace"), QStringLiteral("80")}};
    }
    if (filterName == QLatin1String("FilterDomain"))
    {
        return {{QStringLiteral("name"), QString()},
                {QStringLiteral("userContext"), bool1()}};
    }
    if (filterName == QLatin1String("FilterDun"))
    {
        return {{QStringLiteral("type"), QString()}};
    }
    if (filterName == QLatin1String("FilterFile"))
    {
        return {{QStringLiteral("folder"), bool0()},
                {QStringLiteral("gte"), bool0()},
                {QStringLiteral("lte"), bool0()},
                {QStringLiteral("path"), QString()},
                {QStringLiteral("type"), QStringLiteral("EXISTS")}};
    }
    if (filterName == QLatin1String("FilterGroup"))
    {
        return {{QStringLiteral("localGroup"), bool0()},
                {QStringLiteral("name"), QString()},
                {QStringLiteral("primaryGroup"), bool0()},
                {QStringLiteral("sid"), QString()},
                {QStringLiteral("userContext"), bool1()}};
    }
    if (filterName == QLatin1String("FilterIpRange"))
    {
        return {{QStringLiteral("min"), QStringLiteral("0.0.0.0")},
                {QStringLiteral("max"), QStringLiteral("0.0.0.0")}};
    }
    if (filterName == QLatin1String("FilterLanguage"))
    {
        return {{QStringLiteral("default"), bool0()},
                {QStringLiteral("displayName"), QString()},
                {QStringLiteral("language"), QStringLiteral("9")},
                {QStringLiteral("locale"), QStringLiteral("9")},
                {QStringLiteral("native"), bool0()},
                {QStringLiteral("system"), bool0()}};
    }
    if (filterName == QLatin1String("FilterLdap"))
    {
        return {{QStringLiteral("attribute"), QString()},
                {QStringLiteral("binding"), QString()},
                {QStringLiteral("searchFilter"), QString()},
                {QStringLiteral("variableName"), QString()}};
    }
    if (filterName == QLatin1String("FilterMacRange"))
    {
        return {{QStringLiteral("min"), QStringLiteral("00:00:00:00:00:00")},
                {QStringLiteral("max"), QStringLiteral("00:00:00:00:00:00")}};
    }
    if (filterName == QLatin1String("FilterMsi"))
    {
        return {{QStringLiteral("code"), QString()},
                {QStringLiteral("subtype"), QStringLiteral("EXISTS")},
                {QStringLiteral("type"), QStringLiteral("PRODUCT")}};
    }
    if (filterName == QLatin1String("FilterOrgUnit"))
    {
        return {{QStringLiteral("directMember"), bool0()},
                {QStringLiteral("name"), QString()},
                {QStringLiteral("userContext"), bool1()}};
    }
    if (filterName == QLatin1String("FilterOs"))
    {
        return {{QStringLiteral("class"), QStringLiteral("NE")},
                {QStringLiteral("edition"), QStringLiteral("NE")},
                {QStringLiteral("sp"), QStringLiteral("NE")},
                {QStringLiteral("type"), QStringLiteral("NE")},
                {QStringLiteral("version"), QStringLiteral("NE")}};
    }
    if (filterName == QLatin1String("FilterPortable"))
    {
        return {{QStringLiteral("docked"), bool0()},
                {QStringLiteral("undocked"), bool0()},
                {QStringLiteral("unknown"), bool0()}};
    }
    if (filterName == QLatin1String("FilterProcMode"))
    {
        return processingDefaults();
    }
    if (filterName == QLatin1String("FilterRam"))
    {
        return {{QStringLiteral("totalMB"), QStringLiteral("1024")}};
    }
    if (filterName == QLatin1String("FilterRegistry"))
    {
        return {{QStringLiteral("gte"), bool0()},
                {QStringLiteral("hive"), QStringLiteral("HKEY_LOCAL_MACHINE")},
                {QStringLiteral("key"), QString()},
                {QStringLiteral("lte"), bool0()},
                {QStringLiteral("subtype"), QStringLiteral("EQUALHEX")},
                {QStringLiteral("type"), QStringLiteral("KEYEXISTS")},
                {QStringLiteral("valueType"), QString()}};
    }
    if (filterName == QLatin1String("FilterSite"))
    {
        return {{QStringLiteral("name"), QString()}};
    }
    if (filterName == QLatin1String("FilterTerminal"))
    {
        return {{QStringLiteral("max"), QString()},
                {QStringLiteral("min"), QString()},
                {QStringLiteral("option"), QStringLiteral("NE")},
                {QStringLiteral("type"), QStringLiteral("NE")},
                {QStringLiteral("value"), QString()}};
    }
    if (filterName == QLatin1String("FilterTime"))
    {
        return {{QStringLiteral("begin"), QStringLiteral("00:00")},
                {QStringLiteral("end"), QStringLiteral("23:59")}};
    }
    if (filterName == QLatin1String("FilterUser"))
    {
        return {{QStringLiteral("name"), QString()},
                {QStringLiteral("sid"), QString()}};
    }
    if (filterName == QLatin1String("FilterVariable"))
    {
        return {{QStringLiteral("variableName"), QString()}};
    }
    if (filterName == QLatin1String("FilterWmi"))
    {
        return {{QStringLiteral("nameSpace"), QStringLiteral("root\\cimv2")},
                {QStringLiteral("property"), QString()},
                {QStringLiteral("query"), QString()},
                {QStringLiteral("variableName"), QString()}};
    }

    return {};
}

QMap<QString, QString> materializeTargetingFilterExtras(const QString &filterName,
                                                        const QMap<QString, QString> &extras)
{
    QMap<QString, QString> merged = defaultTargetingFilterExtras(filterName);
    for (auto it = extras.cbegin(); it != extras.cend(); ++it)
    {
        merged.insert(it.key(), it.value());
    }

    if (filterName == QLatin1String("FilterDate"))
    {
        normalizeDateExtras(merged);
    }
    else if (filterName == QLatin1String("FilterProcMode"))
    {
        normalizeProcessingExtras(merged);
    }

    return merged;
}

} // namespace preferences
