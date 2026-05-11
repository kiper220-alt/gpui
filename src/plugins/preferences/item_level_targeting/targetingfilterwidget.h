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

#ifndef GPUI_TARGETING_FILTER_WIDGET_H
#define GPUI_TARGETING_FILTER_WIDGET_H

#include "common/basepreferencewidget.h"

#include <QMap>
#include <QString>
#include <QStringList>

namespace ModelView { class SessionItem; }

namespace preferences
{

class TargetingFilterItem;

//! Common base for the per-filter editor widgets used inside `TargetingDialog`.
//!
//! Each concrete subclass is keyed on an XSD `Filter*` element and binds the
//! attributes from `targetingschema.xsd` to its Designer fields (task 4.2.x
//! / 4.3 in tasks.md).
//!
//! The targeting dialog instantiates the widget via `TargetingWidgetFactory`
//! and feeds it a `TargetingFilterItem*` through `setItem`. Subclasses
//! implement `readFromExtras` / `writeToExtras` to map between their UI
//! controls and the filter's attribute map. Unknown attributes that arrived
//! from the loaded XML are preserved verbatim by the merge step in `submit`,
//! so MSAD round-trip stays byte-stable (D7 in design.md).
//!
//! The IFilter base attributes (`bool`, `not`, `hidden`) are NOT handled by
//! these widgets — they live on the dialog's tree row (combinator / negate /
//! disabled) and are persisted directly on `TargetingFilterItem`.
class TargetingFilterWidget : public BasePreferenceWidget
{
public:
    Q_OBJECT

public:
    explicit TargetingFilterWidget(QWidget *parent = nullptr);
    ~TargetingFilterWidget() override;

    //! XSD element name this widget edits, e.g. "FilterComputer".
    virtual QString xsdName() const = 0;

    //! Bind to a `TargetingFilterItem`. Calls `readFromExtras` with the
    //! current attribute snapshot and remembers it so unknown attributes
    //! can be re-emitted by `submit`.
    void setItem(ModelView::SessionItem *item) override;

    //! Default name shown by `BasePreferenceWidget::name()` callers.
    QString name() const override;

    bool validate() override;

public slots:
    //! Merge the widget's known attributes back onto the item, preserving
    //! any unknown attributes captured at load time.
    void submit() override;

protected:
    //! XSD attribute names this widget owns. The merge step in `submit`
    //! removes these keys from the preserved snapshot before overlaying
    //! the values returned by `writeToExtras`.
    virtual QStringList knownKeys() const = 0;

    //! Populate UI fields from the attribute map (called from `setItem`).
    virtual void readFromExtras(const QMap<QString, QString> &extras) = 0;

    //! Snapshot of the widget's UI fields as XSD attributes. Only the
    //! `knownKeys()` keys must be present.
    virtual QMap<QString, QString> writeToExtras() const = 0;

    //! Convenience accessor for subclasses that need the current item.
    TargetingFilterItem *item() const { return m_item; }

private:
    TargetingFilterWidget(const TargetingFilterWidget &)            = delete;
    TargetingFilterWidget(TargetingFilterWidget &&)                 = delete;
    TargetingFilterWidget &operator=(const TargetingFilterWidget &) = delete;
    TargetingFilterWidget &operator=(TargetingFilterWidget &&)      = delete;

    //! Hook every input child widget (`QLineEdit`, `QCheckBox`,
    //! `QRadioButton`, `QComboBox`, `QSpinBox`, `QDoubleSpinBox`,
    //! `QDateTimeEdit`, `QPlainTextEdit`) to call `submit()` on its
    //! value-changed signal so the inline editor pane in
    //! `TargetingDialog` commits live without an Apply button.
    //! Idempotent: safe to call multiple times.
    void installLiveCommitHooks();

    TargetingFilterItem *m_item{nullptr};
    QMap<QString, QString> m_originalExtras;
    bool m_liveCommitInstalled{false};
};

} // namespace preferences

#endif // GPUI_TARGETING_FILTER_WIDGET_H
