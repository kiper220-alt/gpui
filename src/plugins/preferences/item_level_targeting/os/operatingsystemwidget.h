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

#ifndef GPUI_OPERATING_SYSTEM_FILTER_WIDGET_H
#define GPUI_OPERATING_SYSTEM_FILTER_WIDGET_H

#include "targetingfilterwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class OperatingSystemWidget; }
QT_END_NAMESPACE

namespace preferences
{

//! Editor for `<FilterOs>` (XSD attrs: class, version, type, edition, sp).
//!
//! The Designer combo boxes are populated by `setupUi` from the XSD enums
//! (`enumFilterOsVersion`, `enumFilterOsEdition`, `enumFilterOsSp`,
//! `enumFilterOsType`). The `class` attribute has no Designer field and is
//! preserved as part of the widget-owned extras.
class OperatingSystemFilterWidget : public TargetingFilterWidget
{
public:
    Q_OBJECT

public:
    explicit OperatingSystemFilterWidget(QWidget *parent = nullptr);
    ~OperatingSystemFilterWidget() override;

    QString xsdName() const override { return QStringLiteral("FilterOs"); }

protected:
    QStringList knownKeys() const override;
    void readFromExtras(const QMap<QString, QString> &extras) override;
    QMap<QString, QString> writeToExtras() const override;

private:
    OperatingSystemFilterWidget(const OperatingSystemFilterWidget &)            = delete;
    OperatingSystemFilterWidget(OperatingSystemFilterWidget &&)                 = delete;
    OperatingSystemFilterWidget &operator=(const OperatingSystemFilterWidget &) = delete;
    OperatingSystemFilterWidget &operator=(OperatingSystemFilterWidget &&)      = delete;

    void populateProducts();
    void populateDependentCombos(const QString &editionValue = {},
                                 const QString &servicePackValue = {},
                                 const QString &roleValue = {});

    Ui::OperatingSystemWidget *ui{nullptr};
    QString m_osClass{QStringLiteral("NE")};
};

} // namespace preferences

#endif // GPUI_OPERATING_SYSTEM_FILTER_WIDGET_H
