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

#include "targetingwidgetfactory.h"

#include "common/basepreferencewidget.h"

#include "batterypresentwidget.h"
#include "computernamewidget.h"
#include "cpuspeedwidget.h"
#include "datematchwidget.h"
#include "diskspacewidget.h"
#include "domainfilterwidget.h"
#include "environmentvariablewidget.h"
#include "filematchwidget.h"
#include "ipaddressrangewidget.h"
#include "languagewidget.h"
#include "ldapfilterwidget.h"
#include "macaddressrangewidget.h"
#include "msiquerywidget.h"
#include "networkconnectionwidget.h"
#include "operatingsystemwidget.h"
#include "organizationunitwidget.h"
#include "pcmciapresentwidget.h"
#include "portablecomputerwidget.h"
#include "processingmodewidget.h"
#include "ramfilterwidget.h"
#include "registrymatchwidget.h"
#include "securitygroupwidget.h"
#include "sitefilterwidget.h"
#include "terminalsessionwidget.h"
#include "timerangefilterwidget.h"
#include "userfilterwidget.h"
#include "wmiquerywidget.h"

#include <algorithm>

namespace preferences
{

namespace
{
template <typename T>
TargetingWidgetFactory::Builder makeBuilder()
{
    return []() { return std::unique_ptr<BasePreferenceWidget>(new T()); };
}
} // namespace

TargetingWidgetFactory::TargetingWidgetFactory()
{
    registerWidget(QStringLiteral("FilterBattery"),  makeBuilder<BatteryPresentWidget>());
    registerWidget(QStringLiteral("FilterComputer"), makeBuilder<ComputerNameFilterWidget>());
    registerWidget(QStringLiteral("FilterCpu"),      makeBuilder<CpuSpeedFilterWidget>());
    registerWidget(QStringLiteral("FilterDate"),     makeBuilder<DateMatchFilterWidget>());
    registerWidget(QStringLiteral("FilterDisk"),     makeBuilder<DiskSpaceFilterWidget>());
    registerWidget(QStringLiteral("FilterDomain"),   makeBuilder<DomainFilterWidget>());
    // FilterDun is the MSAD "Network Connection" item — same XSD type, no
    // separate "Dial-Up Networking" widget is shipped (see tasks 4.2.28).
    registerWidget(QStringLiteral("FilterDun"),      makeBuilder<NetworkConnectionFilterWidget>());
    registerWidget(QStringLiteral("FilterFile"),     makeBuilder<FileMatchFilterWidget>());
    registerWidget(QStringLiteral("FilterGroup"),    makeBuilder<SecurityGroupFilterWidget>());
    registerWidget(QStringLiteral("FilterIpRange"),  makeBuilder<IpAddressRangeFilterWidget>());
    registerWidget(QStringLiteral("FilterLanguage"), makeBuilder<LanguageFilterWidget>());
    registerWidget(QStringLiteral("FilterLdap"),     makeBuilder<LdapFilterWidget>());
    registerWidget(QStringLiteral("FilterMacRange"), makeBuilder<MacAddressRangeFilterWidget>());
    registerWidget(QStringLiteral("FilterMsi"),      makeBuilder<MsiQueryFilterWidget>());
    registerWidget(QStringLiteral("FilterOrgUnit"),  makeBuilder<OrganizationUnitFilterWidget>());
    registerWidget(QStringLiteral("FilterOs"),       makeBuilder<OperatingSystemFilterWidget>());
    registerWidget(QStringLiteral("FilterPcmcia"),   makeBuilder<PcmciaPresentWidget>());
    registerWidget(QStringLiteral("FilterPortable"), makeBuilder<PortableComputerFilterWidget>());
    registerWidget(QStringLiteral("FilterProcMode"), makeBuilder<ProcessingModeFilterWidget>());
    registerWidget(QStringLiteral("FilterRam"),      makeBuilder<RamFilterWidget>());
    registerWidget(QStringLiteral("FilterRegistry"), makeBuilder<RegistryMatchFilterWidget>());
    registerWidget(QStringLiteral("FilterSite"),     makeBuilder<SiteFilterWidget>());
    registerWidget(QStringLiteral("FilterTerminal"), makeBuilder<TerminalSessionFilterWidget>());
    registerWidget(QStringLiteral("FilterTime"),     makeBuilder<TimeRangeFilterWidget>());
    registerWidget(QStringLiteral("FilterUser"),     makeBuilder<UserFilterWidget>());
    registerWidget(QStringLiteral("FilterVariable"), makeBuilder<EnvironmentVariableFilterWidget>());
    registerWidget(QStringLiteral("FilterWmi"),      makeBuilder<WmiQueryFilterWidget>());
}

TargetingWidgetFactory::~TargetingWidgetFactory() = default;

void TargetingWidgetFactory::registerWidget(const QString &xsdName, Builder builder)
{
    if (xsdName.isEmpty() || !builder)
    {
        return;
    }

    m_builders.insert(xsdName, std::move(builder));
}

bool TargetingWidgetFactory::hasWidget(const QString &xsdName) const
{
    return m_builders.contains(xsdName);
}

std::unique_ptr<BasePreferenceWidget> TargetingWidgetFactory::create(const QString &xsdName) const
{
    const auto it = m_builders.constFind(xsdName);
    if (it == m_builders.cend())
    {
        return nullptr;
    }

    return it.value()();
}

QStringList TargetingWidgetFactory::registeredNames() const
{
    QStringList names = m_builders.keys();
    std::sort(names.begin(), names.end());
    return names;
}

} // namespace preferences
