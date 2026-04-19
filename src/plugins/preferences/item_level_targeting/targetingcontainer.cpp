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

#include "targetingcontainer.h"

#include <QSharedData>

namespace preferences
{

class TargetingContainerData : public QSharedData
{
public:
    QList<TargetingFilterRecord> filters;
};

namespace
{
bool containsRunOnce(const QList<TargetingFilterRecord> &filters)
{
    for (const auto &f : filters)
    {
        if (f.name == QLatin1String("FilterRunOnce"))
            return true;
        if (!f.children.isEmpty() && containsRunOnce(f.children))
            return true;
    }
    return false;
}
} // namespace

TargetingContainer::TargetingContainer()
    : d(new TargetingContainerData())
{
}

TargetingContainer::TargetingContainer(const TargetingContainer &other) = default;
TargetingContainer::TargetingContainer(TargetingContainer &&other) noexcept = default;
TargetingContainer::~TargetingContainer() = default;

TargetingContainer &TargetingContainer::operator=(const TargetingContainer &other) = default;
TargetingContainer &TargetingContainer::operator=(TargetingContainer &&other) noexcept = default;

bool TargetingContainer::isEmpty() const
{
    return d->filters.isEmpty();
}

const QList<TargetingFilterRecord> &TargetingContainer::filters() const
{
    return d->filters;
}

void TargetingContainer::setFilters(QList<TargetingFilterRecord> filters)
{
    d->filters = std::move(filters);
}

bool TargetingContainer::hasRunOnce() const
{
    return containsRunOnce(d->filters);
}

} // namespace preferences
