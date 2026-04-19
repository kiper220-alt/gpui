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

namespace preferences
{

//! Icon lookup for targeting filters (task 8.2).
//!
//! Maps each MSAD XSD filter element name (e.g. `FilterComputer`) onto a
//! Freedesktop icon-theme name resolved via `QIcon::fromTheme`. Using the
//! system theme rather than shipping our own bitmaps keeps gpui's GUI
//! consistent with the rest of the desktop and avoids the asset-licensing
//! question for a feature that is editor-only.
//!
//! Icons returned by `iconFor()` can be null if the current theme does
//! not ship the mapped name — Qt handles null icons gracefully by just
//! not drawing them, and `QIcon::fromTheme` with a fallback name gives us
//! a predictable result even then.
class TargetingIcons
{
public:
    //! Return the icon for a given XSD filter element name, falling back
    //! to a generic icon when the name is unknown.
    static QIcon iconFor(const QString &xsdName);

    //! Name used for the filter-list rows shown in the tree.
    //! Identical to `iconFor` but exposed separately so callers
    //! documenting intent stay readable.
    static QIcon iconForRow(const QString &xsdName) { return iconFor(xsdName); }
};

} // namespace preferences

#endif // GPUI_TARGETING_ICONS_H
