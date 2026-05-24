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

#ifndef GPUI_TARGETING_WIDGET_FACTORY_H
#define GPUI_TARGETING_WIDGET_FACTORY_H

#include <QHash>
#include <QString>
#include <QStringList>

#include <functional>
#include <memory>

namespace preferences
{

class BasePreferenceWidget;

//! Builds the per-filter editor widget for the targeting dialog (D4 in design.md).
//!
//! Maps XSD element names ("FilterComputer", "FilterBattery", ...) to a
//! builder that constructs the matching `BasePreferenceWidget` subclass
//! from the `.ui` template under `item_level_targeting/`.
//!
//! The factory is intentionally separate from `preferences::WidgetFactory`
//! (which is keyed on top-level Preferences item types like "DrivesItem"):
//! the targeting widgets are leaves in the per-filter editor, not snap-in
//! tabs, and there is exactly one widget class per XSD element.
//!
//! Task 4.1 ships the factory shell; the per-filter widget registrations
//! land with task 4.2.x as their `.cpp/.h` pairs are added.
class TargetingWidgetFactory final
{
public:
    using Builder = std::function<std::unique_ptr<BasePreferenceWidget>()>;

    //! Construct an empty factory and register every known filter widget
    //! whose `.cpp/.h` backing currently exists in the tree.
    TargetingWidgetFactory();
    ~TargetingWidgetFactory();

    //! Add or replace the builder for `xsdName`.
    void registerWidget(const QString &xsdName, Builder builder);

    //! Whether `xsdName` has a registered builder.
    bool hasWidget(const QString &xsdName) const;

    //! Build the widget for `xsdName` or return `nullptr` if unknown.
    //! Caller takes ownership.
    std::unique_ptr<BasePreferenceWidget> create(const QString &xsdName) const;

    //! XSD element names with a registered builder, in sorted order.
    QStringList registeredNames() const;

private:
    TargetingWidgetFactory(const TargetingWidgetFactory &)            = delete;
    TargetingWidgetFactory(TargetingWidgetFactory &&)                 = delete;
    TargetingWidgetFactory &operator=(const TargetingWidgetFactory &) = delete;
    TargetingWidgetFactory &operator=(TargetingWidgetFactory &&)      = delete;

    QHash<QString, Builder> m_builders;
};

} // namespace preferences

#endif // GPUI_TARGETING_WIDGET_FACTORY_H
