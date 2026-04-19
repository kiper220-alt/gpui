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

#include "targetingicons.h"

#include <QHash>

namespace preferences
{

namespace
{

const QHash<QString, QString> &iconTable()
{
    // Freedesktop icon-theme names chosen per XSD filter element.
    // Names follow the Icon Naming Specification; when a filter has no
    // obvious match we pick the closest semantic equivalent.
    static const QHash<QString, QString> table = {
        {QStringLiteral("FilterBattery"),   QStringLiteral("battery")},
        {QStringLiteral("FilterComputer"),  QStringLiteral("computer")},
        {QStringLiteral("FilterCpu"),       QStringLiteral("cpu")},
        {QStringLiteral("FilterDate"),      QStringLiteral("x-office-calendar")},
        {QStringLiteral("FilterDisk"),      QStringLiteral("drive-harddisk")},
        {QStringLiteral("FilterDomain"),    QStringLiteral("network-server")},
        {QStringLiteral("FilterDun"),       QStringLiteral("modem")},
        {QStringLiteral("FilterVariable"),  QStringLiteral("applications-utilities")},
        {QStringLiteral("FilterFile"),      QStringLiteral("text-x-generic")},
        {QStringLiteral("FilterGroup"),     QStringLiteral("system-users")},
        {QStringLiteral("FilterIpRange"),   QStringLiteral("network-wired")},
        {QStringLiteral("FilterLanguage"),  QStringLiteral("preferences-desktop-locale")},
        {QStringLiteral("FilterLdap"),      QStringLiteral("folder-remote")},
        {QStringLiteral("FilterMacRange"),  QStringLiteral("network-wired")},
        {QStringLiteral("FilterMsi"),       QStringLiteral("system-software-install")},
        {QStringLiteral("FilterOrgUnit"),   QStringLiteral("folder")},
        {QStringLiteral("FilterOs"),        QStringLiteral("computer")},
        {QStringLiteral("FilterPcmcia"),    QStringLiteral("media-flash")},
        {QStringLiteral("FilterPortable"),  QStringLiteral("computer-laptop")},
        {QStringLiteral("FilterProcMode"),  QStringLiteral("system-run")},
        {QStringLiteral("FilterRam"),       QStringLiteral("media-flash")},
        {QStringLiteral("FilterRegistry"),  QStringLiteral("preferences-system")},
        {QStringLiteral("FilterSite"),      QStringLiteral("network-workgroup")},
        {QStringLiteral("FilterTerminal"),  QStringLiteral("utilities-terminal")},
        {QStringLiteral("FilterTime"),      QStringLiteral("appointment-soon")},
        {QStringLiteral("FilterUser"),      QStringLiteral("avatar-default")},
        {QStringLiteral("FilterWmi"),       QStringLiteral("system-search")},
        {QStringLiteral("FilterCollection"), QStringLiteral("folder")},
        {QStringLiteral("FilterRunOnce"),   QStringLiteral("media-playlist-repeat")},
    };
    return table;
}

} // namespace

QIcon TargetingIcons::iconFor(const QString &xsdName)
{
    const auto &table = iconTable();
    const auto it = table.constFind(xsdName);
    const QString themeName = (it != table.constEnd())
                                  ? it.value()
                                  : QStringLiteral("dialog-question");
    return QIcon::fromTheme(themeName, QIcon::fromTheme(QStringLiteral("dialog-question")));
}

} // namespace preferences
