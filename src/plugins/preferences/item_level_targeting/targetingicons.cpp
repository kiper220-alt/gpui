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

#include <QApplication>
#include <QHash>
#include <QPair>

namespace preferences
{

namespace
{

struct FilterIconSpec
{
    QStringList            themeNames;
    QStyle::StandardPixmap fallback;
};

const QHash<QString, FilterIconSpec> &filterTable()
{
    // Each filter declares a fallback chain of Freedesktop icon-theme
    // names plus a Qt StandardPixmap used when none of those resolve.
    // Adwaita uses `-symbolic` suffixes; resolve() tries both forms.
    static const QHash<QString, FilterIconSpec> table = {
        {QStringLiteral("FilterBattery"),
            {{QStringLiteral("battery-good"), QStringLiteral("battery")},
             QStyle::SP_DriveHDIcon}},
        {QStringLiteral("FilterComputer"),
            {{QStringLiteral("computer")}, QStyle::SP_ComputerIcon}},
        {QStringLiteral("FilterCpu"),
            {{QStringLiteral("system-run"), QStringLiteral("applications-system"),
              QStringLiteral("cpu")}, QStyle::SP_ComputerIcon}},
        {QStringLiteral("FilterDate"),
            {{QStringLiteral("office-calendar"), QStringLiteral("x-office-calendar"),
              QStringLiteral("appointment-soon")}, QStyle::SP_FileIcon}},
        {QStringLiteral("FilterDisk"),
            {{QStringLiteral("drive-harddisk")}, QStyle::SP_DriveHDIcon}},
        {QStringLiteral("FilterDomain"),
            {{QStringLiteral("network-server"), QStringLiteral("network-workgroup")},
             QStyle::SP_DriveNetIcon}},
        {QStringLiteral("FilterDun"),
            {{QStringLiteral("modem"), QStringLiteral("network-wired")},
             QStyle::SP_DriveNetIcon}},
        {QStringLiteral("FilterVariable"),
            {{QStringLiteral("applications-utilities"), QStringLiteral("utilities-terminal")},
             QStyle::SP_FileIcon}},
        {QStringLiteral("FilterFile"),
            {{QStringLiteral("text-x-generic"), QStringLiteral("document-new")},
             QStyle::SP_FileIcon}},
        {QStringLiteral("FilterGroup"),
            {{QStringLiteral("system-users"), QStringLiteral("user-group")},
             QStyle::SP_DirIcon}},
        {QStringLiteral("FilterIpRange"),
            {{QStringLiteral("network-wired"), QStringLiteral("network-server")},
             QStyle::SP_DriveNetIcon}},
        {QStringLiteral("FilterLanguage"),
            {{QStringLiteral("preferences-desktop-locale"), QStringLiteral("preferences-system")},
             QStyle::SP_FileDialogContentsView}},
        {QStringLiteral("FilterLdap"),
            {{QStringLiteral("folder-remote"), QStringLiteral("network-server")},
             QStyle::SP_DirIcon}},
        {QStringLiteral("FilterMacRange"),
            {{QStringLiteral("network-wired"), QStringLiteral("network-server")},
             QStyle::SP_DriveNetIcon}},
        {QStringLiteral("FilterMsi"),
            {{QStringLiteral("system-software-install"), QStringLiteral("package-install"),
              QStringLiteral("package-x-generic")}, QStyle::SP_FileIcon}},
        {QStringLiteral("FilterOrgUnit"),
            {{QStringLiteral("folder")}, QStyle::SP_DirIcon}},
        {QStringLiteral("FilterOs"),
            {{QStringLiteral("computer"), QStringLiteral("preferences-system")},
             QStyle::SP_ComputerIcon}},
        {QStringLiteral("FilterPcmcia"),
            {{QStringLiteral("media-removable"), QStringLiteral("media-flash"),
              QStringLiteral("drive-removable-media")}, QStyle::SP_DriveHDIcon}},
        {QStringLiteral("FilterPortable"),
            {{QStringLiteral("computer-laptop"), QStringLiteral("computer")},
             QStyle::SP_ComputerIcon}},
        {QStringLiteral("FilterProcMode"),
            {{QStringLiteral("system-run"), QStringLiteral("applications-system")},
             QStyle::SP_BrowserReload}},
        {QStringLiteral("FilterRam"),
            {{QStringLiteral("media-memory"), QStringLiteral("applications-system"),
              QStringLiteral("media-flash")}, QStyle::SP_DriveHDIcon}},
        {QStringLiteral("FilterRegistry"),
            {{QStringLiteral("preferences-system"), QStringLiteral("preferences-other")},
             QStyle::SP_FileDialogDetailedView}},
        {QStringLiteral("FilterSite"),
            {{QStringLiteral("network-workgroup"), QStringLiteral("network-server")},
             QStyle::SP_DriveNetIcon}},
        {QStringLiteral("FilterTerminal"),
            {{QStringLiteral("utilities-terminal"), QStringLiteral("terminal")},
             QStyle::SP_FileIcon}},
        {QStringLiteral("FilterTime"),
            {{QStringLiteral("appointment-new"), QStringLiteral("appointment-soon"),
              QStringLiteral("x-office-calendar")}, QStyle::SP_FileIcon}},
        {QStringLiteral("FilterUser"),
            {{QStringLiteral("system-users"), QStringLiteral("avatar-default"),
              QStringLiteral("user-info")}, QStyle::SP_DirIcon}},
        {QStringLiteral("FilterWmi"),
            {{QStringLiteral("edit-find"), QStringLiteral("system-search")},
             QStyle::SP_FileDialogContentsView}},
        {QStringLiteral("FilterCollection"),
            {{QStringLiteral("folder")}, QStyle::SP_DirIcon}},
        {QStringLiteral("FilterRunOnce"),
            {{QStringLiteral("media-playlist-repeat"), QStringLiteral("media-playback-start")},
             QStyle::SP_MediaPlay}},
    };
    return table;
}

const QHash<QString, FilterIconSpec> &toolbarTable()
{
    static const QHash<QString, FilterIconSpec> table = {
        {QStringLiteral("list-add"),
            {{QStringLiteral("list-add"), QStringLiteral("document-new")},
             QStyle::SP_FileDialogNewFolder}},
        {QStringLiteral("edit-delete"),
            {{QStringLiteral("edit-delete"), QStringLiteral("user-trash")},
             QStyle::SP_TrashIcon}},
        {QStringLiteral("go-up"),
            {{QStringLiteral("go-up"), QStringLiteral("arrow-up")},
             QStyle::SP_ArrowUp}},
        {QStringLiteral("go-down"),
            {{QStringLiteral("go-down"), QStringLiteral("arrow-down")},
             QStyle::SP_ArrowDown}},
        {QStringLiteral("edit-cut"),
            {{QStringLiteral("edit-cut")}, QStyle::SP_DialogDiscardButton}},
        {QStringLiteral("edit-copy"),
            {{QStringLiteral("edit-copy")}, QStyle::SP_FileIcon}},
        {QStringLiteral("edit-paste"),
            {{QStringLiteral("edit-paste")}, QStyle::SP_FileLinkIcon}},
        {QStringLiteral("folder-new"),
            {{QStringLiteral("folder-new"), QStringLiteral("folder")},
             QStyle::SP_FileDialogNewFolder}},
        {QStringLiteral("edit-clear"),
            {{QStringLiteral("edit-clear"), QStringLiteral("edit-delete")},
             QStyle::SP_DialogResetButton}},
        {QStringLiteral("edit-undo"),
            {{QStringLiteral("edit-undo"), QStringLiteral("process-stop")},
             QStyle::SP_DialogCancelButton}},
        {QStringLiteral("document-properties"),
            {{QStringLiteral("document-properties"), QStringLiteral("preferences-system")},
             QStyle::SP_FileDialogDetailedView}},
    };
    return table;
}

} // namespace

QIcon TargetingIcons::resolve(const QStringList &themeNames, QStyle::StandardPixmap fallback)
{
    for (const QString &name : themeNames)
    {
        // Adwaita-style symbolic name first, then plain.
        const QString symbolic = name + QStringLiteral("-symbolic");
        if (QIcon::hasThemeIcon(symbolic))
        {
            return QIcon::fromTheme(symbolic);
        }
        if (QIcon::hasThemeIcon(name))
        {
            return QIcon::fromTheme(name);
        }
    }
    if (auto *style = QApplication::style())
    {
        return style->standardIcon(fallback);
    }
    return QIcon();
}

QIcon TargetingIcons::iconFor(const QString &xsdName)
{
    const auto &table = filterTable();
    const auto it = table.constFind(xsdName);
    if (it != table.constEnd())
    {
        return resolve(it.value().themeNames, it.value().fallback);
    }
    return resolve({}, QStyle::SP_MessageBoxQuestion);
}

QIcon TargetingIcons::toolbarIcon(const QString &xsdName)
{
    const auto &table = toolbarTable();
    const auto it = table.constFind(xsdName);
    if (it != table.constEnd())
    {
        return resolve(it.value().themeNames, it.value().fallback);
    }
    return resolve({xsdName}, QStyle::SP_FileIcon);
}

} // namespace preferences
