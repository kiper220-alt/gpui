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

#include "targetingfilteritem.h"

#include <mvvm/model/taginfo.h>

#include <QCoreApplication>

namespace preferences
{

const std::string TargetingFilterItem::kModelType      = "TargetingFilterItem";
const std::string TargetingFilterItem::kPropName       = "name";
const std::string TargetingFilterItem::kPropId         = "id";
const std::string TargetingFilterItem::kPropCombinator = "combinator";
const std::string TargetingFilterItem::kPropNegated    = "negated";
const std::string TargetingFilterItem::kPropDisabled   = "disabled";
const std::string TargetingFilterItem::kChildrenTag    = "children";

TargetingFilterItem::TargetingFilterItem()
    : ModelView::CompoundItem(kModelType)
{
    // mvvm rejects empty QVariants in addProperty, so store strings as
    // std::string (matching CommonItem) and convert at the accessor boundary.
    addProperty(kPropName, "");
    addProperty(kPropId, "");
    addProperty(kPropCombinator, "AND");
    addProperty(kPropNegated, false);
    addProperty(kPropDisabled, false);

    registerTag(ModelView::TagInfo::universalTag(kChildrenTag, {kModelType}));

    setEditable(false);
}

QString TargetingFilterItem::filterName() const
{
    return QString::fromStdString(property<std::string>(kPropName));
}

void TargetingFilterItem::setFilterName(const QString &name)
{
    setProperty(kPropName, name.toStdString());
    refreshDisplayName();
}

QString TargetingFilterItem::filterId() const
{
    return QString::fromStdString(property<std::string>(kPropId));
}

void TargetingFilterItem::setFilterId(const QString &id)
{
    setProperty(kPropId, id.toStdString());
}

QString TargetingFilterItem::combinator() const
{
    return QString::fromStdString(property<std::string>(kPropCombinator));
}

void TargetingFilterItem::setCombinator(const QString &combinator)
{
    setProperty(kPropCombinator, combinator.toStdString());
    refreshDisplayName();
}

bool TargetingFilterItem::negated() const
{
    return property<bool>(kPropNegated);
}

void TargetingFilterItem::setNegated(bool value)
{
    setProperty(kPropNegated, value);
    refreshDisplayName();
}

bool TargetingFilterItem::disabled() const
{
    return property<bool>(kPropDisabled);
}

void TargetingFilterItem::setDisabled(bool value)
{
    setProperty(kPropDisabled, value);
}

bool TargetingFilterItem::isCollection() const
{
    return filterName() == QLatin1String("FilterCollection");
}

const QMap<QString, QString> &TargetingFilterItem::extras() const
{
    return m_extras;
}

void TargetingFilterItem::setExtras(QMap<QString, QString> extras)
{
    m_extras = std::move(extras);
}

void TargetingFilterItem::loadRecord(const TargetingFilterRecord &record)
{
    setFilterName(record.name);
    setFilterId(record.id);
    setCombinator(record.combinator);
    setNegated(record.negated);
    setDisabled(record.disabled);
    setExtras(record.attributes);
    refreshDisplayName();

    while (itemCount(kChildrenTag) > 0)
    {
        takeItem({kChildrenTag, 0});
    }

    for (const auto &child : record.children)
    {
        auto *node = insertItem<TargetingFilterItem>({kChildrenTag, itemCount(kChildrenTag)});
        node->loadRecord(child);
    }
}

TargetingFilterRecord TargetingFilterItem::toRecord() const
{
    TargetingFilterRecord record;
    record.name       = filterName();
    record.id         = filterId();
    record.combinator = combinator();
    record.negated    = negated();
    record.disabled   = disabled();
    record.attributes = m_extras;

    for (auto *child : items<TargetingFilterItem>(kChildrenTag))
    {
        record.children.append(child->toRecord());
    }
    return record;
}

QString TargetingFilterItem::label() const
{
    const QString fullName = filterName();
    const QString baseName = fullName.startsWith(QLatin1String("Filter"))
                                 ? fullName.mid(6)
                                 : fullName;

    QString prefix = combinator();
    if (negated())
    {
        prefix += QStringLiteral(" NOT");
    }

    const QString body = isCollection()
                             ? QCoreApplication::translate("TargetingFilterItem", "Collection")
                             : baseName;
    return prefix.trimmed().isEmpty() ? body : prefix.trimmed() + QLatin1Char(' ') + body;
}

void TargetingFilterItem::refreshDisplayName()
{
    setDisplayName(label().toStdString());
}

} // namespace preferences
