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

#ifndef GPUI_TARGETING_DIALOG_H
#define GPUI_TARGETING_DIALOG_H

#include "targetingcontainer.h"

#include <QDialog>
#include <QHash>
#include <QPoint>
#include <QString>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui { class TargetingDialog; }
class QAction;
class QComboBox;
class QIdentityProxyModel;
class QModelIndex;
QT_END_NAMESPACE

namespace ModelView
{
class SessionItem;
class TagRow;
}
namespace preferences { class BasePreferenceWidget; }

namespace preferences
{

class TargetingFilterItem;
class TargetingModel;
class TargetingWidgetFactory;

//! MSAD-style item-level targeting editor.
//!
//! The dialog hosts a tree view backed by `TargetingModel` (a wrapper over
//! a `ModelView::SessionModel` of `TargetingFilterItem`s) and exposes the
//! standard toolbar actions: New Item, Delete, Move Up/Down, Cut/Copy/
//! Paste, Wrap/Unwrap collection, Negate, AND/OR combinator, per-filter
//! Options. Per-filter property editors are built on demand from
//! `TargetingWidgetFactory`.
class TargetingDialog : public QDialog
{
public:
    Q_OBJECT

public:
    explicit TargetingDialog(QWidget *parent = nullptr);
    ~TargetingDialog() override;

    void setContainer(const TargetingContainer &container);
    TargetingContainer container() const;

    //! MSAD filter catalog (XSD element names), minus FilterRunOnce
    //! which is reserved for the Apply-once checkbox.
    static QStringList filterCatalog();
    static QString filterDisplayName(const QString &filterName);

private slots:
    // Toolbar / context menu actions.
    void addFilter(const QString &filterName);
    void addCollection();
    void deleteSelected();
    void moveUp();
    void moveDown();
    void cutSelected();
    void copySelected();
    void pasteAtSelection();
    void wrapInCollection();
    void unwrapCollection();
    void toggleNegate();
    void setCombinatorAnd();
    void setCombinatorOr();

    // Selection-driven UI state.
    void onTreeContextMenu(const QPoint &pos);
    void onSelectionChanged();

    // Item Options compound widget handlers.
    void onCombinatorComboChanged(int index);
    void onNegatedComboChanged(int index);

private:
    TargetingDialog(const TargetingDialog&)            = delete;
    TargetingDialog(TargetingDialog&&)                 = delete;
    TargetingDialog& operator=(const TargetingDialog&) = delete;
    TargetingDialog& operator=(TargetingDialog&&)      = delete;

    void buildToolbar();
    void refreshLabels(TargetingFilterItem *root = nullptr);
    void finishMutation(TargetingFilterItem *refreshRoot = nullptr);
    void updateActionStates();

    //! Show the inline editor pane for `item` (or the placeholder when
    //! `item` is null). Caches one editor widget per filter type inside
    //! `editorStack` so re-selecting a row of the same type reuses the
    //! cached widget instead of rebuilding it.
    void showInlineEditor(TargetingFilterItem *item);

    //! Compute the (parent, insertRow, tag) triple where a freshly
    //! created top-level filter or collection should land, based on the
    //! current selection (matches the slot logic from the previous
    //! `addFilter` implementation).
    void computeInsertionSlot(ModelView::SessionItem *&parent,
                              int &insertRow,
                              std::string &tag) const;

    //! Items for the current selection (deduplicated by SessionItem).
    QList<TargetingFilterItem *> selectedItems() const;
    void copyItemsToClipboard(const QList<TargetingFilterItem *> &items) const;
    void removeItems(const QList<TargetingFilterItem *> &items);

    //! `selectedItems()` filtered to entries that share a single parent.
    QList<TargetingFilterItem *> selectedSiblings() const;
    QList<TargetingFilterItem *> selectedContiguousSiblings() const;
    TargetingFilterItem *itemFromProxyIndex(const QModelIndex &index) const;
    QModelIndex proxyIndexForItem(TargetingFilterItem *item) const;
    void selectItem(TargetingFilterItem *item);
    void selectItems(const QList<TargetingFilterItem *> &items);

    Ui::TargetingDialog *ui {nullptr};
    std::unique_ptr<TargetingModel> m_model;
    std::unique_ptr<TargetingWidgetFactory> m_widgetFactory;
    QIdentityProxyModel *m_proxy{nullptr};

    QAction *m_actDelete{nullptr};
    QAction *m_actUp{nullptr};
    QAction *m_actDown{nullptr};
    QAction *m_actCut{nullptr};
    QAction *m_actCopy{nullptr};
    QAction *m_actPaste{nullptr};
    QAction *m_actAddCollection{nullptr};

    // Kept as members (not on toolbar anymore) — surfaced through the
    // row-context menu only.
    QAction *m_actWrap{nullptr};
    QAction *m_actUnwrap{nullptr};
    QAction *m_actNegate{nullptr};
    QAction *m_actCombAnd{nullptr};
    QAction *m_actCombOr{nullptr};

    // Item Options compound (combinator + negate dropdowns on the toolbar).
    QComboBox *m_combCombinator{nullptr};
    QComboBox *m_combNegated{nullptr};

    // Cache of per-filter editor widgets in editorStack, keyed by XSD
    // filter element name. Built lazily on first selection of that type.
    QHash<QString, BasePreferenceWidget *> m_editorCache;
};

} // namespace preferences

#endif // GPUI_TARGETING_DIALOG_H
