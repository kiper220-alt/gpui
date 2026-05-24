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

#ifndef GPUI_TARGETING_WRITER_H
#define GPUI_TARGETING_WRITER_H

#include "targetingcontainer.h"

class QDomDocument;
class QDomElement;

namespace preferences
{

//! Serialises a TargetingFilterRecord into a DOM element.
//!
//! Generic targeting attributes are written directly from the record. Runtime
//! identity attributes such as id/uid are intentionally not emitted to XML.
//! FilterCollection records are recursed; their children are written as
//! sub-elements.
//!
//! Returns false only for an empty element name.
bool writeFilterRecord(QDomDocument &doc,
                       QDomElement &parent,
                       const TargetingFilterRecord &record);

//! Convenience: write an entire TargetingContainer as a <Filters> element.
void writeTargetingContainer(QDomDocument &doc,
                             QDomElement &parent,
                             const TargetingContainer &container);

} // namespace preferences

#endif // GPUI_TARGETING_WRITER_H
