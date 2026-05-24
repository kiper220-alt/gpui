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

#include "targetingrowformatter.h"

#include "../os/oscatalog.h"

#include <QCoreApplication>
#include <QHash>
#include <QString>
#include <QStringList>
#include <functional>

namespace preferences
{

namespace
{

#define TR(text) QCoreApplication::translate("TargetingRowFormatter", text)

Q_DECL_UNUSED const char *const kTargetingRowFormatterTranslations[] = {
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "(unset)"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "battery is present"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "PCMCIA is present"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "computer is a portable"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "computer name is %1"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "CPU speed is at least %1 MHz"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "RAM is at least %1 MB"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "free space on %1 is at least %2 MB"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "date matches %1 (period %2)"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "time is between %1 and %2"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "computer is in domain %1"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "computer is in organizational unit %1"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "computer is in site %1"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "file %1 version is between %2 and %3"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "file %1 exists"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "user is member of group %1"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "user is %1"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "IP address is in range %1 – %2"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "MAC address is in range %1 – %2"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "language is %1"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "language code is %1"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "User"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "System"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "Native"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", " or "),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "MSI %1 matches"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "operating system"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "operating system %1"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", ", role %1"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "Windows XP"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "Windows Server 2003"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "Windows Server 2003 R2"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "Windows Vista"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "Windows Server 2008"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "Windows 7"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "Windows Server 2008 R2"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "Windows 8"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "Windows Server 2012 Family"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "Windows 8.1"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "Windows Server 2012 R2 Family"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "Windows 10"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "Windows Server 2019 Family"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "any"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "processing mode matches"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "processing mode: %1"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "synchronous foreground"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "asynchronous foreground"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "background refresh"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "forced refresh"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "link transition"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "no changes"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "RSoP transition"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "safe boot"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "slow link"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "verbose logging"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "registry value %1 in %2\\%3 matches"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "registry key %1\\%2 exists"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "terminal session"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "terminal session: %1"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "Remote Desktop Services"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "Application name"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "Client name"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "Initial program"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "Session name"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "Working directory"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "Client TCP/IP address"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "dial-up connection"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "dial-up connection %1"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "Telephone modem accessed through a COM port"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "ISDN card with corresponding NDISWAN driver installed"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "X.25 card with corresponding NDISWAN driver installed"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "Virtual Private Network (VPN)"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "Packet Assembler/Disassembler"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "Generic"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "Serial Port direct connection"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "Frame Relay"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "Asynchronous Transfer Mode (ATM)"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "Sonet"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "Switched 56K access"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "Infrared Data Association (IrDA) device"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "Parallel Port direct connection"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "PPP over Ethernet (PPPoE)"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "environment variable %%1%% is %2"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "WMI query \"%1\" matches"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "LDAP query against %1 matches"),
    QT_TRANSLATE_NOOP("TargetingRowFormatter", "collection"),
};

QString unsetPlaceholder()
{
    return TR("(unset)");
}

QString attr(const QMap<QString, QString> &m, const QString &key)
{
    const auto it = m.find(key);
    return (it != m.end() && !it.value().isEmpty()) ? it.value() : unsetPlaceholder();
}

bool isAnyChoice(const QString &value)
{
    return value.isEmpty() || value == QLatin1String("NE");
}

QString choiceLabel(const QList<OsChoice> &choices, const QString &value)
{
    if (isAnyChoice(value))
    {
        return {};
    }
    for (const auto &choice : choices)
    {
        if (choice.value == value)
        {
            return choice.label;
        }
    }
    return value;
}

QString osProductLabel(const QString &value)
{
    if (isAnyChoice(value))
    {
        return {};
    }

    // Row text needs a grammatical form independent from combo-box labels.
    static const QHash<QString, const char *> labels = {
        {QStringLiteral("XP"),              "Windows XP"},
        {QStringLiteral("2K3"),             "Windows Server 2003"},
        {QStringLiteral("2K3R2"),           "Windows Server 2003 R2"},
        {QStringLiteral("VISTA"),           "Windows Vista"},
        {QStringLiteral("2K8"),             "Windows Server 2008"},
        {QStringLiteral("WIN7"),            "Windows 7"},
        {QStringLiteral("2K8R2"),           "Windows Server 2008 R2"},
        {QStringLiteral("WIN8"),            "Windows 8"},
        {QStringLiteral("WIN8S"),           "Windows Server 2012 Family"},
        {QStringLiteral("WINBLUE"),         "Windows 8.1"},
        {QStringLiteral("WINBLUESRV"),      "Windows Server 2012 R2 Family"},
        {QStringLiteral("WINTHRESHOLD"),    "Windows 10"},
        {QStringLiteral("WINTHRESHOLDSRV"), "Windows Server 2019 Family"},
    };

    const auto it = labels.constFind(value);
    if (it != labels.constEnd())
    {
        return TR(*it);
    }

    const auto product = OsCatalog::productByValue(value);
    return product.value == value ? product.label : value;
}

QString formatOperatingSystem(const QMap<QString, QString> &e)
{
    const QString productValue = e.value(QStringLiteral("version"));
    const auto product = OsCatalog::productByValue(productValue);

    QStringList subjectParts;
    const QString productLabel = osProductLabel(productValue);
    if (!productLabel.isEmpty())
    {
        subjectParts << productLabel;
    }

    const QString editionLabel = choiceLabel(product.editions, e.value(QStringLiteral("edition")));
    if (!editionLabel.isEmpty())
    {
        subjectParts << editionLabel;
    }

    const QString spLabel = choiceLabel(product.servicePacks, e.value(QStringLiteral("sp")));
    if (!spLabel.isEmpty())
    {
        subjectParts << spLabel;
    }

    QString text = subjectParts.isEmpty()
        ? TR("operating system")
        : TR("operating system %1").arg(subjectParts.join(QLatin1Char(' ')));

    const QString roleLabel = choiceLabel(product.roles, e.value(QStringLiteral("type")));
    if (!roleLabel.isEmpty())
    {
        text += TR(", role %1").arg(roleLabel);
    }

    return text;
}

QString labelFromMap(const QHash<QString, const char *> &labels, const QString &value)
{
    if (isAnyChoice(value))
    {
        return {};
    }
    const auto it = labels.constFind(value);
    return it != labels.constEnd() ? TR(*it) : value;
}

QString formatProcessingMode(const QMap<QString, QString> &e)
{
    const QList<QPair<QString, const char *>> flags = {
        {QStringLiteral("syncFore"),   "synchronous foreground"},
        {QStringLiteral("asyncFore"),  "asynchronous foreground"},
        {QStringLiteral("backRefr"),   "background refresh"},
        {QStringLiteral("forceRefr"),  "forced refresh"},
        {QStringLiteral("linkTrns"),   "link transition"},
        {QStringLiteral("noChg"),      "no changes"},
        {QStringLiteral("rsopTrns"),   "RSoP transition"},
        {QStringLiteral("safeBoot"),   "safe boot"},
        {QStringLiteral("slowLink"),   "slow link"},
        {QStringLiteral("verbLog"),    "verbose logging"},
    };

    QStringList enabled;
    for (const auto &flag : flags)
    {
        if (e.value(flag.first) == QLatin1String("1"))
        {
            enabled << TR(flag.second);
        }
    }

    return enabled.isEmpty() ? TR("processing mode matches")
                             : TR("processing mode: %1").arg(enabled.join(TR(" or ")));
}

QString formatTerminalSession(const QMap<QString, QString> &e)
{
    static const QHash<QString, const char *> types = {
        {QStringLiteral("TS"), "Remote Desktop Services"},
    };
    static const QHash<QString, const char *> options = {
        {QStringLiteral("APPLICATION"), "Application name"},
        {QStringLiteral("CLIENT"),      "Client name"},
        {QStringLiteral("PROGRAM"),     "Initial program"},
        {QStringLiteral("SESSION"),     "Session name"},
        {QStringLiteral("DIRECTORY"),   "Working directory"},
        {QStringLiteral("IP"),          "Client TCP/IP address"},
    };

    QStringList parts;
    const QString type = labelFromMap(types, e.value(QStringLiteral("type")));
    if (!type.isEmpty())
    {
        parts << type;
    }
    const QString option = labelFromMap(options, e.value(QStringLiteral("option")));
    if (!option.isEmpty())
    {
        parts << option;
    }
    const QString value = e.value(QStringLiteral("value"));
    if (!value.isEmpty())
    {
        parts << value;
    }

    return parts.isEmpty() ? TR("terminal session")
                           : TR("terminal session: %1").arg(parts.join(QStringLiteral(", ")));
}

QString formatNetworkConnection(const QMap<QString, QString> &e)
{
    static const QHash<QString, const char *> types = {
        {QStringLiteral("modem"),      "Telephone modem accessed through a COM port"},
        {QStringLiteral("isdn"),       "ISDN card with corresponding NDISWAN driver installed"},
        {QStringLiteral("x25"),        "X.25 card with corresponding NDISWAN driver installed"},
        {QStringLiteral("vpn"),        "Virtual Private Network (VPN)"},
        {QStringLiteral("pad"),        "Packet Assembler/Disassembler"},
        {QStringLiteral("GENERIC"),    "Generic"},
        {QStringLiteral("SERIAL"),     "Serial Port direct connection"},
        {QStringLiteral("FRAMERELAY"), "Frame Relay"},
        {QStringLiteral("ATM"),        "Asynchronous Transfer Mode (ATM)"},
        {QStringLiteral("SONET"),      "Sonet"},
        {QStringLiteral("SW56"),       "Switched 56K access"},
        {QStringLiteral("IRDA"),       "Infrared Data Association (IrDA) device"},
        {QStringLiteral("PARALLEL"),   "Parallel Port direct connection"},
        {QStringLiteral("PPPoE"),      "PPP over Ethernet (PPPoE)"},
    };

    const QString label = labelFromMap(types, e.value(QStringLiteral("type")));
    return label.isEmpty() ? TR("dial-up connection")
                           : TR("dial-up connection %1").arg(label);
}

using Formatter = std::function<QString(const QMap<QString, QString> &)>;

const QHash<QString, Formatter> &table()
{
    static const QHash<QString, Formatter> t = {
        {QStringLiteral("FilterBattery"),
            [](const QMap<QString, QString> &) {
                return TR("battery is present");
            }},
        {QStringLiteral("FilterPcmcia"),
            [](const QMap<QString, QString> &) {
                return TR("PCMCIA is present");
            }},
        {QStringLiteral("FilterPortable"),
            [](const QMap<QString, QString> &) {
                return TR("computer is a portable");
            }},
        {QStringLiteral("FilterComputer"),
            [](const QMap<QString, QString> &e) {
                return TR("computer name is %1").arg(attr(e, QStringLiteral("name")));
            }},
        {QStringLiteral("FilterCpu"),
            [](const QMap<QString, QString> &e) {
                return TR("CPU speed is at least %1 MHz").arg(attr(e, QStringLiteral("speedMHz")));
            }},
        {QStringLiteral("FilterRam"),
            [](const QMap<QString, QString> &e) {
                return TR("RAM is at least %1 MB").arg(attr(e, QStringLiteral("totalMB")));
            }},
        {QStringLiteral("FilterDisk"),
            [](const QMap<QString, QString> &e) {
                return TR("free space on %1 is at least %2 MB")
                    .arg(attr(e, QStringLiteral("drive")),
                         attr(e, QStringLiteral("freeSpace")));
            }},
        {QStringLiteral("FilterDate"),
            [](const QMap<QString, QString> &e) {
                return TR("date matches %1 (period %2)")
                    .arg(attr(e, QStringLiteral("dow")),
                         attr(e, QStringLiteral("period")));
            }},
        {QStringLiteral("FilterTime"),
            [](const QMap<QString, QString> &e) {
                return TR("time is between %1 and %2")
                    .arg(attr(e, QStringLiteral("begin")), attr(e, QStringLiteral("end")));
            }},
        {QStringLiteral("FilterDomain"),
            [](const QMap<QString, QString> &e) {
                return TR("computer is in domain %1").arg(attr(e, QStringLiteral("name")));
            }},
        {QStringLiteral("FilterOrgUnit"),
            [](const QMap<QString, QString> &e) {
                return TR("computer is in organizational unit %1")
                    .arg(attr(e, QStringLiteral("name")));
            }},
        {QStringLiteral("FilterSite"),
            [](const QMap<QString, QString> &e) {
                return TR("computer is in site %1").arg(attr(e, QStringLiteral("name")));
            }},
        {QStringLiteral("FilterFile"),
            [](const QMap<QString, QString> &e) {
                const QString type = e.value(QStringLiteral("type"), QStringLiteral("EXISTS"));
                if (type == QLatin1String("VERSION"))
                {
                    return TR("file %1 version is between %2 and %3")
                        .arg(attr(e, QStringLiteral("path")),
                             attr(e, QStringLiteral("min")),
                             attr(e, QStringLiteral("max")));
                }
                return TR("file %1 exists").arg(attr(e, QStringLiteral("path")));
            }},
        {QStringLiteral("FilterGroup"),
            [](const QMap<QString, QString> &e) {
                return TR("user is member of group %1").arg(attr(e, QStringLiteral("name")));
            }},
        {QStringLiteral("FilterUser"),
            [](const QMap<QString, QString> &e) {
                return TR("user is %1").arg(attr(e, QStringLiteral("name")));
            }},
        {QStringLiteral("FilterIpRange"),
            [](const QMap<QString, QString> &e) {
                return TR("IP address is in range %1 – %2")
                    .arg(attr(e, QStringLiteral("min")), attr(e, QStringLiteral("max")));
            }},
        {QStringLiteral("FilterMacRange"),
            [](const QMap<QString, QString> &e) {
                return TR("MAC address is in range %1 – %2")
                    .arg(attr(e, QStringLiteral("min")), attr(e, QStringLiteral("max")));
            }},
        {QStringLiteral("FilterLanguage"),
            [](const QMap<QString, QString> &e) {
                const QString name = e.value(QStringLiteral("displayName"));
                QString head;
                if (!name.isEmpty())
                {
                    head = TR("language is %1").arg(name);
                }
                else
                {
                    head = TR("language code is %1").arg(attr(e, QStringLiteral("language")));
                }
                // Append the User/System/Native scope flags joined with " or ".
                QStringList flags;
                if (e.value(QStringLiteral("default")) == QLatin1String("1"))
                {
                    flags << TR("User");
                }
                if (e.value(QStringLiteral("system")) == QLatin1String("1"))
                {
                    flags << TR("System");
                }
                if (e.value(QStringLiteral("native")) == QLatin1String("1"))
                {
                    flags << TR("Native");
                }
                if (flags.isEmpty())
                {
                    return head;
                }
                return QStringLiteral("%1 (%2)").arg(head, flags.join(TR(" or ")));
            }},
        {QStringLiteral("FilterMsi"),
            [](const QMap<QString, QString> &e) {
                return TR("MSI %1 matches").arg(attr(e, QStringLiteral("code")));
            }},
        {QStringLiteral("FilterOs"),
            [](const QMap<QString, QString> &e) {
                return formatOperatingSystem(e);
            }},
        {QStringLiteral("FilterProcMode"),
            [](const QMap<QString, QString> &e) {
                return formatProcessingMode(e);
            }},
        {QStringLiteral("FilterRegistry"),
            [](const QMap<QString, QString> &e) {
                const QString hive = e.value(QStringLiteral("hive"));
                const QString key  = attr(e, QStringLiteral("key"));
                const QString val  = e.value(QStringLiteral("valueName"));
                const QString fullKey = key.isEmpty() ? hive : (hive + QStringLiteral("\\") + key);
                if (!val.isEmpty())
                {
                    return TR("registry value %1 in %2 matches").arg(val, fullKey);
                }
                return TR("registry key %1 exists").arg(fullKey);
            }},
        {QStringLiteral("FilterTerminal"),
            [](const QMap<QString, QString> &e) {
                return formatTerminalSession(e);
            }},
        {QStringLiteral("FilterDun"),
            [](const QMap<QString, QString> &e) {
                return formatNetworkConnection(e);
            }},
        {QStringLiteral("FilterVariable"),
            [](const QMap<QString, QString> &e) {
                return TR("environment variable %%1%% is %2")
                    .arg(attr(e, QStringLiteral("variableName")),
                         attr(e, QStringLiteral("value")));
            }},
        {QStringLiteral("FilterWmi"),
            [](const QMap<QString, QString> &e) {
                return TR("WMI query \"%1\" matches").arg(attr(e, QStringLiteral("query")));
            }},
        {QStringLiteral("FilterLdap"),
            [](const QMap<QString, QString> &e) {
                return TR("LDAP query against %1 matches")
                    .arg(attr(e, QStringLiteral("binding")));
            }},
    };
    return t;
}

} // namespace

QString TargetingRowFormatter::format(const QString &filterName,
                                      const QMap<QString, QString> &extras)
{
    const auto it = table().constFind(filterName);
    if (it != table().constEnd())
    {
        return (it.value())(extras);
    }
    if (filterName == QLatin1String("FilterCollection"))
    {
        // Pure-data overload can't query the live child count.
        return TR("collection");
    }
    // Fallback: strip the "Filter" prefix.
    return filterName.startsWith(QLatin1String("Filter")) ? filterName.mid(6) : filterName;
}

// `format(const TargetingFilterItem *)` is implemented in
// `targetingfilteritem.cpp` so this translation unit stays free of any
// MVVM linkage. That lets the targeting tests compile only the pure
// data side without dragging the MVVM library into the test target.

} // namespace preferences
