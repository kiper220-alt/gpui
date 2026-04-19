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

#ifndef GPUI_FILTERS_BRIDGE_H
#define GPUI_FILTERS_BRIDGE_H

#include "targetingcontainer.h"

#include <QString>

class QDomDocument;
class QDomElement;

namespace preferences
{

//! Convert a `<Filters>` DOM element into a `TargetingContainer`.
//! Element/attribute names and order are preserved verbatim — every
//! direct child of `<Filters>` becomes a `TargetingFilterRecord` whose
//! `name` is the element local-name and whose `attributes` map is built
//! from the element's attributes in source order. `FilterCollection`
//! children are recursed; other elements are treated as leaves.
TargetingContainer containerFromXml(const QDomElement &filtersElement);

//! Append a `<Filters>` element representing `container` as a child of
//! `parent`. Attribute order is the iteration order of
//! `TargetingFilterRecord::attributes` (insertion-stable in QMap by key).
//! Records with empty `name` are skipped.
void appendContainerToXml(QDomDocument &doc,
                          QDomElement &parent,
                          const TargetingContainer &container);

} // namespace preferences

#endif // GPUI_FILTERS_BRIDGE_H
