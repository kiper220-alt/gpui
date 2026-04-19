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

#ifndef GPUI_TARGETING_MODEL_H
#define GPUI_TARGETING_MODEL_H

#include "targetingcontainer.h"

#include <QObject>
#include <memory>

namespace ModelView
{
class SessionItem;
class SessionModel;
class ViewModel;
} // namespace ModelView

namespace preferences
{

class TargetingFilterItem;

//! Adapter between `TargetingContainer` and the mvvm `SessionModel` that
//! drives the targeting editor.
//!
//! Owns:
//!   * A `ModelView::SessionModel` whose root has a single "filters" tag
//!     populated with `TargetingFilterItem` nodes (and their nested
//!     children for `FilterCollection`).
//!   * A `ModelView::ViewModel` (`TopItemsViewModel`) that exposes the
//!     filter tree to Qt's `QTreeView` while hiding property children.
//!
//! `FilterRunOnce` entries on the source container are stripped out and
//! re-emitted untouched on save — their lifecycle is driven by the
//! Common-tab "Apply once" checkbox (section 6 of tasks.md).
class TargetingModel : public QObject
{
    Q_OBJECT

public:
    //! Top-level tag on the SessionModel root; holds the editable filters.
    static const std::string kFiltersTag;

    explicit TargetingModel(QObject *parent = nullptr);
    ~TargetingModel() override;

    //! Replace the model contents from a container.
    void fromContainer(const TargetingContainer &container);

    //! Serialize the tree back to a container.
    //! Hidden `FilterRunOnce` entries from load are re-emitted at the head.
    TargetingContainer toContainer() const;

    //! Underlying SessionModel — passed to per-filter widgets so they can
    //! build a `Factory::CreatePropertyFlatViewModel(...)` over it.
    ModelView::SessionModel *sessionModel() const;

    //! View model used as the source for the dialog's `QTreeView`.
    ModelView::ViewModel *viewModel() const;

    //! Root item that owns the top-level filter rows.
    ModelView::SessionItem *rootFilterParent() const;

    //! Top-level filter items.
    QList<TargetingFilterItem *> topFilters() const;

private:
    TargetingModel(const TargetingModel &)            = delete;
    TargetingModel(TargetingModel &&)                 = delete;
    TargetingModel &operator=(const TargetingModel &) = delete;
    TargetingModel &operator=(TargetingModel &&)      = delete;

    void clearFilters();

    std::unique_ptr<ModelView::SessionModel> m_data;
    std::unique_ptr<ModelView::ViewModel> m_view;

    QList<TargetingFilterRecord> m_hiddenRunOnce;
};

} // namespace preferences

#endif // GPUI_TARGETING_MODEL_H
