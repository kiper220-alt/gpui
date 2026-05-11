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

#ifndef GPUI_FILTERS_IO_H
#define GPUI_FILTERS_IO_H

#include "targetingcontainer.h"

#include <istream>
#include <map>
#include <string>

namespace preferences
{

class PreferencesModel;

//! Per-sub-plugin entry point for `<Filters>` round-trip.
//!
//! Each preference sub-plugin (drives, files, folders, ini, registry,
//! shares, shortcuts, variables, ...) calls this helper from its
//! `*PreferenceReader::createModel` and `*PreferenceWriter::writeModel`
//! to keep filter handling colocated with xsd-cxx parsing — and out of
//! the base reader/writer where it would be invisible to the sub-plugin
//! author.
//!
//! Two sides:
//!   * **Read**: `stripFilters` removes every `<Filters>` subtree from
//!     the raw XML before xsd-cxx sees it (xsd-cxx is strict about the
//!     XSD's `Filters_t` and would fail the whole document if a filter
//!     attribute is missing). Returns the cleaned XML plus a map of
//!     parsed filters indexed by the parent element's `uid` attribute.
//!     After the sub-plugin builds its model, call `applyToModel` to
//!     attach each parsed container to its `CommonItem`.
//!   * **Write**: `injectFilters` takes XML produced by xsd-cxx and
//!     appends a `<Filters>` child to every parent whose matching
//!     `CommonItem` carries filters or apply-once. xsd-cxx-emitted
//!     `<Filters>` blocks are dropped first to keep the output single-
//!     source-of-truth.
class FiltersIO
{
public:
    using FiltersMap = std::map<std::string, TargetingContainer>;

    struct StripResult
    {
        //! XML with every `<Filters>` element removed; safe to feed to
        //! xsd-cxx generated parsers.
        std::string cleanedXml;

        //! Parsed filters indexed by the `uid` attribute of the element
        //! that carried the original `<Filters>` child.
        FiltersMap filters;
    };

    //! Slurp `input` into a string, strip every `<Filters>` subtree,
    //! parse them via the DOM bridge.
    static StripResult stripFilters(std::istream &input);

    //! Walk every `CommonItem` registered under tag `"common"` in the
    //! model; for each one whose `UID` is present in `filters`, call
    //! `setFiltersFromXml` so the load-side bookkeeping (`APPLY_ONCE`,
    //! `RUN_ONCE_ID`) runs.
    static void applyToModel(PreferencesModel *model, const FiltersMap &filters);

    //! Append `<Filters>` children to the XML produced by xsd-cxx,
    //! sourced from `model`'s `CommonItem::filtersForSerialization()`.
    //! Returns the patched XML.
    static std::string injectFilters(const std::string &xml, const PreferencesModel *model);
};

} // namespace preferences

#endif // GPUI_FILTERS_IO_H
