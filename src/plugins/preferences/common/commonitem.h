/***********************************************************************************************************************
**
** Copyright (C) 2021 BaseALT Ltd. <org@basealt.ru>
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

#ifndef GPUI_COMMONITEM_H
#define GPUI_COMMONITEM_H

#include "basepreferenceitem.h"
#include "item_level_targeting/common/targetingcontainer.h"

namespace preferences
{

//! Common item representation for editor.

class CommonItem : public BasePreferenceItem<CommonItem>
{
public:
    enum PropertyType
    {
        CLSID                 = 0,
        DISABLED              = 1,
        NAME                  = 2,
        STATUS                = 3,
        IMAGE                 = 4,
        CHANGED               = 5,
        UID                   = 6,
        DESC                  = 7,
        BYPASS_ERRORS         = 8,
        USER_CONTEXT          = 9,
        REMOVE_POLICY         = 10,
        APPLY_ONCE            = 11,
        ITEM_LEVEL_TARGETING  = 12,
        FILTERS               = 13,
        RUN_ONCE_ID           = 14,
    };

public:
    CommonItem();
    CommonItem(const CommonItem &other);

    QString name() const;
    void setName(const QString& path);

    std::string changed() const;
    void setChanged(bool state);

    QString desc() const;
    void setDesc(const QString& path);

    bool bypassErrors() const;
    void setBypassErrors(bool state);

    bool userContext() const;
    void setUserContext(bool state);

    bool removePolicy() const;
    void setRemovePolicy(bool state);

    bool applyOnce() const;
    void setApplyOnce(bool state);

    bool itemLevelTargeting() const;
    void setItemLevelTargeting(bool state);

    //! Opaque container for this item's <Filters> subtree. Not exposed
    //! through the MVVM property system (custom type not supported by
    //! the QVariant variant registry).
    //!
    //! `filters()` returns the user-visible filter list — `FilterRunOnce`
    //! entries are stripped because the Common-tab "Apply once" checkbox
    //! owns that filter (D1 in design.md). Use `filtersForSerialization()`
    //! when writing back to disk to get the same list with the
    //! `FilterRunOnce` entry re-injected at the head.
    TargetingContainer filters() const;

    //! Pure setter used by the targeting-dialog accept path. Stores the
    //! container as-is; never mutates `APPLY_ONCE` or `RUN_ONCE_ID`. The
    //! dialog hides `FilterRunOnce` rows by design (it round-trips the
    //! checkbox state through `filtersForSerialization`), so it never
    //! hands a `FilterRunOnce` back here.
    void setFilters(TargetingContainer filters);

    //! Load-side setter used by `BasePreferenceReader`. Strips any
    //! `FilterRunOnce` entries, caches the first one's `id` into
    //! `RUN_ONCE_ID`, and infers `APPLY_ONCE` from the marker's presence.
    void setFiltersFromXml(TargetingContainer filters);

    //! Container as it should appear on disk: a leading `FilterRunOnce`
    //! when `applyOnce()` is true, otherwise identical to `filters()`.
    TargetingContainer filtersForSerialization() const;

    //! Stable GUID used as the FilterRunOnce `id` attribute when
    //! apply-once is enabled. Generated once at construction and
    //! preserved across load/save.
    std::string runOnceId() const;
    void setRunOnceId(const std::string &id);

    constexpr static inline const char* propertyToString(const PropertyType& type)
    {
        switch (type)
        {
        case CLSID               : return "clsid";
        case DISABLED            : return "disabled";
        case NAME                : return "name";
        case STATUS              : return "status";
        case IMAGE               : return "image";
        case CHANGED             : return "changed";
        case UID                 : return "uid";
        case DESC                : return "desc";
        case BYPASS_ERRORS       : return "bypassErrors";
        case USER_CONTEXT        : return "userContext ";
        case REMOVE_POLICY       : return "removePolicy";
        case APPLY_ONCE          : return "applyOnce";
        case ITEM_LEVEL_TARGETING: return "itemLevelTargeting";
        case FILTERS             : return "filters";
        case RUN_ONCE_ID         : return "runOnceId";
        }
        return "";
    }

private:
    TargetingContainer m_filters;
};

}

Q_DECLARE_METATYPE(::preferences::CommonItem)

#endif//GPUI_COMMONITEM_H
