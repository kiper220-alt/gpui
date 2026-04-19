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

#ifndef GPUI_REGISTRY_MATCH_FILTER_WIDGET_H
#define GPUI_REGISTRY_MATCH_FILTER_WIDGET_H

#include "targetingfilterwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class RegistryMatchWidget; }
QT_END_NAMESPACE

namespace preferences
{

//! Editor for `<FilterRegistry>` (XSD attrs: type, subtype, valueName,
//! valueType, valueData, variableName, key, hive, min, max, gte, lte,
//! version).
//!
//! `registrymatchwidget.ui` swaps panels by `type` (KEYEXISTS / VALUEEXISTS
//! / MATCHVALUE / GETVALUE) and `subtype` (EQUALHEX / EQUALDEC / SUBSTRING
//! / VERSION). The first iteration round-trips attributes verbatim — the
//! per-mode editing experience is refined in a later iteration.
class RegistryMatchFilterWidget : public TargetingFilterWidget
{
public:
    Q_OBJECT

public:
    explicit RegistryMatchFilterWidget(QWidget *parent = nullptr);
    ~RegistryMatchFilterWidget() override;

    QString xsdName() const override { return QStringLiteral("FilterRegistry"); }

protected:
    QStringList knownKeys() const override { return {}; }
    void readFromExtras(const QMap<QString, QString> &) override {}
    QMap<QString, QString> writeToExtras() const override { return {}; }

private:
    Ui::RegistryMatchWidget *ui{nullptr};
};

} // namespace preferences

#endif // GPUI_REGISTRY_MATCH_FILTER_WIDGET_H
