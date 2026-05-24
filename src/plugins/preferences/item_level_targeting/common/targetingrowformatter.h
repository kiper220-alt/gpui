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

#ifndef GPUI_TARGETING_ROW_FORMATTER_H
#define GPUI_TARGETING_ROW_FORMATTER_H

#include <QMap>
#include <QString>

namespace preferences
{

class TargetingFilterItem;

//! Humanised, localised row-text generator for the targeting tree.
//!
//! Each XSD filter type contributes a `tr()`-friendly format string that
//! turns its attribute map into a single-line description, e.g.
//! `language is Russian (Russia)` for `FilterLanguage`. The combinator
//! prefix (`AND` / `OR` / blank for the first row) is added by the
//! caller — `TargetingFilterItem::label()` — so per-filter lambdas stay
//! free of combinator concerns.
//!
//! When a filter type has no entry in the formatter table, `format`
//! returns the filter's display name (current behaviour) as a graceful
//! fallback.
class TargetingRowFormatter
{
public:
    //! Returns the humanised, localised, single-line description for
    //! `item`'s current attributes (without combinator prefix).
    static QString format(const TargetingFilterItem *item);

    //! Pure-data overload used by tests and any code that wants to
    //! render a row without holding a live `TargetingFilterItem`.
    //! Filter types whose format depends on live state (e.g. the child
    //! count of `FilterCollection`) fall back to a generic placeholder
    //! when called through this overload.
    static QString format(const QString &filterName, const QMap<QString, QString> &extras);
};

} // namespace preferences

#endif // GPUI_TARGETING_ROW_FORMATTER_H
