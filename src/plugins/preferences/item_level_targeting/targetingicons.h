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

#ifndef GPUI_TARGETING_ICONS_H
#define GPUI_TARGETING_ICONS_H

#include <QIcon>
#include <QString>
#include <QStringList>
#include <QStyle>

namespace preferences
{

//! Icon lookup for targeting filters and toolbar actions.
//!
//! Resolves an icon by walking a list of Freedesktop icon-theme names,
//! trying each both as-is and with a `-symbolic` suffix (Adwaita's
//! preferred form). When no theme name matches the active icon set, a
//! `QStyle::StandardPixmap` is used so something always renders even on
//! systems with a minimal icon-theme install (the user's complaint
//! that drove this layer).
class TargetingIcons
{
public:
    //! Return the icon for a given XSD filter element name.
    static QIcon iconFor(const QString &xsdName);

    //! Toolbar icons (Delete, Cut, Copy, ...) keyed by Freedesktop name.
    //! Falls back to the matching QStyle::StandardPixmap when the active
    //! theme has nothing.
    static QIcon toolbarIcon(const QString &xsdName);

    //! Generic resolver: walk `themeNames`, trying each plain and as
    //! `-symbolic`. If none match, return `QApplication::style()->standardIcon(fallback)`.
    static QIcon resolve(const QStringList &themeNames, QStyle::StandardPixmap fallback);

    //! Per-row tree icon (alias of iconFor for readability).
    static QIcon iconForRow(const QString &xsdName) { return iconFor(xsdName); }
};

} // namespace preferences

#endif // GPUI_TARGETING_ICONS_H
