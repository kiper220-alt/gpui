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

#include "targetingmodel.h"

#include "targetingfilteritem.h"

#include <mvvm/factories/viewmodelfactory.h>
#include <mvvm/model/sessionitem.h>
#include <mvvm/model/sessionmodel.h>
#include <mvvm/model/taginfo.h>
#include <mvvm/viewmodel/topitemsviewmodel.h>

namespace preferences
{

const std::string TargetingModel::kFiltersTag = "filters";

TargetingModel::TargetingModel(QObject *parent)
    : QObject(parent)
    , m_data(std::make_unique<ModelView::SessionModel>("TargetingModel"))
{
    m_data->registerItem<TargetingFilterItem>();
    m_data->rootItem()->registerTag(
        ModelView::TagInfo::universalTag(kFiltersTag, {TargetingFilterItem::kModelType}),
        true);

    m_view = std::make_unique<ModelView::TopItemsViewModel>(m_data.get());
}

TargetingModel::~TargetingModel() = default;

void TargetingModel::fromContainer(const TargetingContainer &container)
{
    clearFilters();

    for (const auto &record : container.filters())
    {
        if (record.name == QLatin1String("FilterRunOnce"))
        {
            m_hiddenRunOnce.append(record);
            continue;
        }
        auto *item = m_data->insertItem<TargetingFilterItem>(m_data->rootItem(),
                                                             {kFiltersTag, -1});
        item->loadRecord(record);
    }
}

TargetingContainer TargetingModel::toContainer() const
{
    QList<TargetingFilterRecord> out;
    out.append(m_hiddenRunOnce);

    for (auto *item : topFilters())
    {
        out.append(item->toRecord());
    }

    TargetingContainer container;
    container.setFilters(std::move(out));
    return container;
}

ModelView::SessionModel *TargetingModel::sessionModel() const
{
    return m_data.get();
}

ModelView::ViewModel *TargetingModel::viewModel() const
{
    return m_view.get();
}

ModelView::SessionItem *TargetingModel::rootFilterParent() const
{
    return m_data->rootItem();
}

QList<TargetingFilterItem *> TargetingModel::topFilters() const
{
    QList<TargetingFilterItem *> out;
    for (auto *child : m_data->rootItem()->getItems(kFiltersTag))
    {
        if (auto *casted = dynamic_cast<TargetingFilterItem *>(child))
        {
            out.append(casted);
        }
    }
    return out;
}

void TargetingModel::clearFilters()
{
    m_hiddenRunOnce.clear();

    auto *root = m_data->rootItem();
    while (root->itemCount(kFiltersTag) > 0)
    {
        m_data->removeItem(root, {kFiltersTag, 0});
    }
}

} // namespace preferences
