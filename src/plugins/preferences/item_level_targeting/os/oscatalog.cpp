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

#include "oscatalog.h"

#include <QCoreApplication>

namespace preferences
{

namespace
{

#define OS_TR(text) QCoreApplication::translate("OperatingSystemFilterWidget", text)

Q_DECL_UNUSED const char *const kOsCatalogTranslations[] = {
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Any"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "No service packs installed"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Windows XP"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Windows Server 2003"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Windows Server 2003 R2"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Windows Vista"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Windows Server 2008"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Windows 7"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Windows Server 2008 R2"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Windows 8"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Windows Server 2012 Family"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Windows 8.1"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Windows Server 2012 R2 Family"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Windows 10"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Windows Server 2019 Family"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "ALT Linux"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Linux"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "64-bit"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Professional"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Media Center"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Tablet PC"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Enterprise"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Datacenter"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Standard"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Web"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Enterprise, 64-bit"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Datacenter, 64-bit"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Standard, 64-bit"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Web, 64-bit"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Business"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Ultimate"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Business, 64-bit"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Ultimate, 64-bit"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "HPC Server"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Itanium-based Systems"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Standard without Hyper-V"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Enterprise without Hyper-V"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Datacenter without Hyper-V"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Standard without Hyper-V, 64-bit"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Enterprise without Hyper-V, 64-bit"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Datacenter without Hyper-V, 64-bit"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Professional, 64-bit"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Datacenter"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Storage Server Standard"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Storage Server Workgroup"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "MultiPoint Server Premium"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Essentials"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Service Pack 1"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Service Pack 2"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Service Pack 3"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Member Server"),
    QT_TRANSLATE_NOOP("OperatingSystemFilterWidget", "Domain Controller"),
};

OsChoice c(const char *label, const char *value)
{
    return {OS_TR(label), QString::fromLatin1(value)};
}

QList<OsChoice> withAny(std::initializer_list<OsChoice> choices)
{
    QList<OsChoice> out;
    out.append(OsCatalog::anyChoice());
    for (const auto &choice : choices)
    {
        out.append(choice);
    }
    return out;
}

QList<OsChoice> baseSp()
{
    return {OsCatalog::anyChoice(), OsCatalog::noServicePacksChoice()};
}

QList<OsChoice> sp(const QList<OsChoice> &extra)
{
    auto out = baseSp();
    out.append(extra);
    return out;
}

QList<OsChoice> desktopRoles()
{
    return {OsCatalog::anyChoice()};
}

QList<OsChoice> serverRoles()
{
    return {OsCatalog::anyChoice(), c("Member Server", "SV"), c("Domain Controller", "DC")};
}

QList<OsChoice> xpEditions()
{
    return withAny({
        c("64-bit", "64"),
        c("Professional", "PRO"),
        c("Media Center", "MC"),
        c("Tablet PC", "TPC"),
    });
}

QList<OsChoice> server2003Editions()
{
    return withAny({
        c("Enterprise, 64-bit", "64EP"),
        c("Datacenter, 64-bit", "64DC"),
        c("Standard, 64-bit", "64"),
        c("Web, 64-bit", "WEB"),
        c("Enterprise", "EP"),
        c("Datacenter", "DTC"),
        c("Standard", "STD"),
        c("Web", "WEB"),
    });
}

QList<OsChoice> vistaEditions()
{
    return withAny({
        c("Enterprise", "EP"),
        c("Business", "PRO"),
        c("Ultimate", "TPC"),
        c("Enterprise, 64-bit", "64EP"),
        c("Business, 64-bit", "64"),
        c("Ultimate, 64-bit", "64"),
    });
}

QList<OsChoice> server2008Editions()
{
    auto out = server2003Editions();
    out.append(c("HPC Server", "SRV"));
    out.append(c("Itanium-based Systems", "64"));
    out.append(c("Standard without Hyper-V", "STD"));
    out.append(c("Enterprise without Hyper-V", "EP"));
    out.append(c("Datacenter without Hyper-V", "DTC"));
    out.append(c("Standard without Hyper-V, 64-bit", "64"));
    out.append(c("Enterprise without Hyper-V, 64-bit", "64EP"));
    out.append(c("Datacenter without Hyper-V, 64-bit", "64DC"));
    return out;
}

QList<OsChoice> win7Editions()
{
    return withAny({
        c("Enterprise", "EP"),
        c("Professional", "PRO"),
        c("Ultimate", "TPC"),
        c("Enterprise, 64-bit", "64EP"),
        c("Professional, 64-bit", "64"),
        c("Ultimate, 64-bit", "64"),
    });
}

QList<OsChoice> win8Editions()
{
    return withAny({
        c("Enterprise", "EP"),
        c("Professional", "PRO"),
        c("Enterprise, 64-bit", "64EP"),
        c("Professional, 64-bit", "64"),
    });
}

QList<OsChoice> server2012Editions()
{
    return withAny({
        c("Datacenter", "DTC"),
        c("Standard", "STD"),
        c("Storage Server Standard", "SRV"),
        c("Storage Server Workgroup", "WS"),
        c("MultiPoint Server Premium", "TSE"),
        c("Essentials", "SBS"),
    });
}

OsProduct p(const char *label, const char *value, const QList<OsChoice> &editions,
            const QList<OsChoice> &servicePacks, const QList<OsChoice> &roles,
            bool visible = true)
{
    return {OS_TR(label), QString::fromLatin1(value), editions, servicePacks, roles, visible};
}

} // namespace

QList<OsProduct> OsCatalog::products()
{
    const QList<OsChoice> sp1 = {c("Service Pack 1", "Service Pack 1")};
    const QList<OsChoice> sp12 = {c("Service Pack 1", "Service Pack 1"),
                                  c("Service Pack 2", "Service Pack 2")};
    const QList<OsChoice> sp123 = {c("Service Pack 1", "Service Pack 1"),
                                   c("Service Pack 2", "Service Pack 2"),
                                   c("Service Pack 3", "Service Pack 3")};

    return {
        p("Any", "NE", withAny({}), baseSp(), desktopRoles()),
        p("Windows XP", "XP", xpEditions(), sp(sp123), desktopRoles()),
        p("Windows Server 2003", "2K3", server2003Editions(), sp(sp12), serverRoles()),
        p("Windows Server 2003 R2", "2K3R2", server2003Editions(), sp(sp12), serverRoles()),
        p("Windows Vista", "VISTA", vistaEditions(), sp(sp1), desktopRoles()),
        p("Windows Server 2008", "2K8", server2008Editions(), baseSp(), serverRoles()),
        p("Windows 7", "WIN7", win7Editions(), baseSp(), desktopRoles()),
        p("Windows Server 2008 R2", "2K8R2", server2008Editions(), baseSp(), serverRoles()),
        p("Windows 8", "WIN8", win8Editions(), baseSp(), desktopRoles()),
        p("Windows Server 2012 Family", "WIN8S", server2012Editions(), baseSp(), serverRoles()),
        p("Windows 8.1", "WINBLUE", win8Editions(), baseSp(), desktopRoles()),
        p("Windows Server 2012 R2 Family", "WINBLUESRV", server2012Editions(), baseSp(), serverRoles()),
        p("Windows 10", "WINTHRESHOLD", win8Editions(), baseSp(), desktopRoles()),
        p("Windows Server 2019 Family", "WINTHRESHOLDSRV", server2012Editions(), baseSp(), serverRoles()),

        // Reserved placeholders: deliberately hidden until schema/value
        // mappings and dependent choices are specified.
        p("ALT Linux", "ALT_LINUX", withAny({}), baseSp(), desktopRoles(), false),
        p("Linux", "LINUX", withAny({}), baseSp(), desktopRoles(), false),
    };
}

QList<OsProduct> OsCatalog::visibleProducts()
{
    QList<OsProduct> out;
    for (const auto &product : products())
    {
        if (product.visible)
        {
            out.append(product);
        }
    }
    return out;
}

QList<OsProduct> OsCatalog::reservedProducts()
{
    QList<OsProduct> out;
    for (const auto &product : products())
    {
        if (!product.visible)
        {
            out.append(product);
        }
    }
    return out;
}

OsProduct OsCatalog::productByValue(const QString &value)
{
    for (const auto &product : products())
    {
        if (product.value == value)
        {
            return product;
        }
    }
    return p("Any", "NE", withAny({}), baseSp(), desktopRoles());
}

OsChoice OsCatalog::anyChoice()
{
    return {OS_TR("Any"), QStringLiteral("NE")};
}

OsChoice OsCatalog::noServicePacksChoice()
{
    return {OS_TR("No service packs installed"), QStringLiteral("Gold")};
}

} // namespace preferences
