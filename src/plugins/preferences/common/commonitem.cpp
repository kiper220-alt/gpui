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

#include "commonitem.h"

#include <QUuid>

namespace preferences
{

CommonItem::CommonItem()
    : BasePreferenceItem<::preferences::CommonItem>("CommonItem")
{
    addProperty(propertyToString(CLSID), "");
    addProperty(propertyToString(DISABLED), false);
    addProperty(propertyToString(NAME), "");
    addProperty(propertyToString(STATUS), "");
    addProperty(propertyToString(IMAGE), static_cast<int>(0));
    addProperty(propertyToString(CHANGED), "");
    addProperty(propertyToString(UID), QUuid::createUuid().toString().toStdString());
    addProperty(propertyToString(DESC), "");
    addProperty(propertyToString(BYPASS_ERRORS), false);
    addProperty(propertyToString(USER_CONTEXT), false);
    addProperty(propertyToString(REMOVE_POLICY), false);
    addProperty(propertyToString(APPLY_ONCE), false);
    addProperty(propertyToString(ITEM_LEVEL_TARGETING), false);
    addProperty(propertyToString(RUN_ONCE_ID), QUuid::createUuid().toString().toStdString());
}

CommonItem::CommonItem(const CommonItem &other)
    : BasePreferenceItem<::preferences::CommonItem>("CommonItem")
    , m_filters(other.m_filters)
{
    copyProperty<std::string>(CLSID, other);
    copyProperty<bool>(DISABLED, other);
    copyProperty<std::string>(NAME, other);
    copyProperty<std::string>(STATUS, other);
    copyProperty<int>(IMAGE, other);
    copyProperty<std::string>(CHANGED, other);
    copyProperty<std::string>(UID, other);
    copyProperty<std::string>(DESC, other);
    copyProperty<bool>(BYPASS_ERRORS, other);
    copyProperty<bool>(USER_CONTEXT, other);
    copyProperty<bool>(REMOVE_POLICY, other);
    copyProperty<bool>(APPLY_ONCE, other);
    copyProperty<bool>(ITEM_LEVEL_TARGETING, other);
    copyProperty<std::string>(RUN_ONCE_ID, other);
}

QString CommonItem::name() const
{
    return property<QString>(propertyToString(NAME));
}

void CommonItem::setName(const QString &newName)
{
    setProperty(propertyToString(NAME), newName);
}

std::string CommonItem::changed() const
{
    return property<std::string>(propertyToString(CHANGED));
}

void CommonItem::setChanged(bool state)
{
    setProperty(propertyToString(CHANGED), state);
}

QString CommonItem::desc() const
{
    return property<QString>(propertyToString(DESC));
}

void CommonItem::setDesc(const QString &newDesc)
{
    setProperty(propertyToString(DESC), newDesc);
}

bool CommonItem::bypassErrors() const
{
    return property<bool>(propertyToString(BYPASS_ERRORS));
}

void CommonItem::setBypassErrors(bool state)
{
    setProperty(propertyToString(BYPASS_ERRORS), state);
}

bool CommonItem::userContext() const
{
    return property<bool>(propertyToString(USER_CONTEXT));
}

void CommonItem::setUserContext(bool state)
{
    setProperty(propertyToString(USER_CONTEXT), state);
}

bool CommonItem::removePolicy() const
{
    return property<bool>(propertyToString(REMOVE_POLICY));
}

void CommonItem::setRemovePolicy(bool state)
{
    setProperty(propertyToString(REMOVE_POLICY), state);
}

bool CommonItem::applyOnce() const
{
    return property<bool>(propertyToString(APPLY_ONCE));
}

void CommonItem::setApplyOnce(bool state)
{
    setProperty(propertyToString(APPLY_ONCE), state);
}

bool CommonItem::itemLevelTargeting() const
{
    return property<bool>(propertyToString(ITEM_LEVEL_TARGETING));
}

void CommonItem::setItemLevelTargeting(bool state)
{
    setProperty(propertyToString(ITEM_LEVEL_TARGETING), state);
}

TargetingContainer CommonItem::filters() const
{
    return m_filters;
}

void CommonItem::setFilters(TargetingContainer filters)
{
    // Apply-once is owned by the Common tab's checkbox. When the incoming
    // container carries a FilterRunOnce entry (load path from XML), strip
    // it, cache the id, and force APPLY_ONCE on. When the container has
    // no FilterRunOnce (UI accept path), leave APPLY_ONCE / RUN_ONCE_ID
    // alone — the checkbox state is authoritative.
    bool seenRunOnce = false;
    QList<TargetingFilterRecord> kept;
    kept.reserve(filters.filters().size());
    for (const auto &record : filters.filters())
    {
        if (record.name == QLatin1String("FilterRunOnce"))
        {
            seenRunOnce = true;
            if (!record.id.isEmpty())
            {
                setRunOnceId(record.id.toStdString());
            }
            continue;
        }
        kept.append(record);
    }

    if (seenRunOnce)
    {
        setApplyOnce(true);
    }
    else if (!filters.filters().isEmpty())
    {
        // Authoritative load with no FilterRunOnce → checkbox should be off.
        // (If `filters` is empty we cannot tell load from a fresh edit, so
        // the checkbox state is preserved.)
        setApplyOnce(false);
    }

    TargetingContainer trimmed;
    trimmed.setFilters(std::move(kept));
    m_filters = std::move(trimmed);
}

TargetingContainer CommonItem::filtersForSerialization() const
{
    auto records = m_filters.filters();
    if (applyOnce())
    {
        TargetingFilterRecord runOnce;
        runOnce.name       = QStringLiteral("FilterRunOnce");
        runOnce.id         = QString::fromStdString(runOnceId());
        runOnce.combinator = QStringLiteral("AND"); // bool="1"
        runOnce.negated    = false;
        records.prepend(runOnce);
    }
    TargetingContainer container;
    container.setFilters(std::move(records));
    return container;
}

std::string CommonItem::runOnceId() const
{
    return property<std::string>(propertyToString(RUN_ONCE_ID));
}

void CommonItem::setRunOnceId(const std::string &id)
{
    setProperty(propertyToString(RUN_ONCE_ID), id);
}

}
