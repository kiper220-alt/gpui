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

#ifndef GPUI_TARGETING_FILTER_ITEM_H
#define GPUI_TARGETING_FILTER_ITEM_H

#include "targetingcontainer.h"

#include <mvvm/model/compounditem.h>

#include <QString>

namespace preferences
{

//! mvvm node for one filter row in the targeting editor (D2/D4 in design.md).
//!
//! Each `TargetingFilterItem` is a `ModelView::CompoundItem` whose registered
//! properties hold the IFilter base attributes (`name`, `id`, `combinator`,
//! `negated`, `disabled`). Per-filter widgets (task 4.2.x) bind editor fields
//! to these properties via `QDataWidgetMapper` over
//! `Factory::CreatePropertyFlatViewModel(item->model())`, the same pattern
//! used by `DrivesWidget`.
//!
//! Filter-specific XSD attributes (e.g. `name` on `FilterComputer`,
//! `min`/`max` on `FilterCpu`) are stored in the per-filter extras map and
//! are accessed by the matching widget class. Unknown attributes encountered
//! on load also live there so they round-trip verbatim (D7).
//!
//! Nested filters (children of `FilterCollection`) are inserted under the
//! `kChildrenTag` universal tag so the `TopItemsViewModel`-driven tree
//! shows them while hiding property children.
class TargetingFilterItem : public ModelView::CompoundItem
{
public:
    static const std::string kModelType;

    //! Property tag names for IFilter base attributes.
    static const std::string kPropName;       //!< QString — XSD element name (e.g. "FilterComputer")
    static const std::string kPropId;         //!< QString — UUID id attribute
    static const std::string kPropCombinator; //!< QString — "AND" / "OR"
    static const std::string kPropNegated;    //!< bool    — IFilter::not
    static const std::string kPropDisabled;   //!< bool    — IFilter::disabled

    //! Tag under which nested filters live (used by FilterCollection).
    static const std::string kChildrenTag;

    TargetingFilterItem();

    //! Convenience accessors mirroring `TargetingFilterRecord` fields.
    QString filterName() const;
    void setFilterName(const QString &name);

    QString filterId() const;
    void setFilterId(const QString &id);

    QString combinator() const;
    void setCombinator(const QString &combinator);

    bool negated() const;
    void setNegated(bool value);

    bool disabled() const;
    void setDisabled(bool value);

    //! Whether this row is the FilterCollection grouping node.
    bool isCollection() const;

    //! Per-filter and unknown XSD attributes preserved verbatim.
    const QMap<QString, QString> &extras() const;
    void setExtras(QMap<QString, QString> extras);

    //! Replace the whole record (including nested children) on this item.
    //! Tree children are recreated from `record.children`.
    void loadRecord(const TargetingFilterRecord &record);

    //! Materialize this item (and its descendants) back to a record.
    TargetingFilterRecord toRecord() const;

    //! Human-readable label shown in the tree (e.g. "AND NOT Computer").
    QString label() const;

    //! Refresh the SessionItem display name from `label()`. Call after any
    //! property change that affects the visible row text.
    void refreshDisplayName();

private:
    QMap<QString, QString> m_extras;
};

} // namespace preferences

#endif // GPUI_TARGETING_FILTER_ITEM_H
