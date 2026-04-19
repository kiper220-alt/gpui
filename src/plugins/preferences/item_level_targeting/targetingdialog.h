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
#include <QPoint>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui { class TargetingDialog; }
class QAction;
QT_END_NAMESPACE

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

private slots:
    // Toolbar / context menu actions.
    void addFilter(const QString &filterName);
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
    void editProperties();

    // Selection-driven UI state.
    void onTreeContextMenu(const QPoint &pos);
    void onSelectionChanged();

private:
    TargetingDialog(const TargetingDialog&)            = delete;
    TargetingDialog(TargetingDialog&&)                 = delete;
    TargetingDialog& operator=(const TargetingDialog&) = delete;
    TargetingDialog& operator=(TargetingDialog&&)      = delete;

    void buildToolbar();
    void refreshLabels(TargetingFilterItem *root = nullptr);
    void updateActionStates();

    //! Items for the current selection (deduplicated by SessionItem).
    QList<TargetingFilterItem *> selectedItems() const;

    //! `selectedItems()` filtered to entries that share a single parent.
    QList<TargetingFilterItem *> selectedSiblings() const;

    Ui::TargetingDialog *ui {nullptr};
    std::unique_ptr<TargetingModel> m_model;
    std::unique_ptr<TargetingWidgetFactory> m_widgetFactory;

    QAction *m_actDelete{nullptr};
    QAction *m_actUp{nullptr};
    QAction *m_actDown{nullptr};
    QAction *m_actCut{nullptr};
    QAction *m_actCopy{nullptr};
    QAction *m_actPaste{nullptr};
    QAction *m_actWrap{nullptr};
    QAction *m_actUnwrap{nullptr};
    QAction *m_actNegate{nullptr};
    QAction *m_actCombAnd{nullptr};
    QAction *m_actCombOr{nullptr};
    QAction *m_actOptions{nullptr};
};

} // namespace preferences

#endif // GPUI_TARGETING_DIALOG_H
