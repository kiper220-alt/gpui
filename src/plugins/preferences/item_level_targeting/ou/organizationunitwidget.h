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

#ifndef GPUI_ORGANIZATION_UNIT_FILTER_WIDGET_H
#define GPUI_ORGANIZATION_UNIT_FILTER_WIDGET_H

#include "targetingfilterwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class OrganizationUnitWidget; }
QT_END_NAMESPACE

namespace preferences
{

//! Editor for `<FilterOrgUnit name=".." userContext="0|1" directMember="0|1"/>`.
class OrganizationUnitFilterWidget : public TargetingFilterWidget
{
public:
    Q_OBJECT

public:
    explicit OrganizationUnitFilterWidget(QWidget *parent = nullptr);
    ~OrganizationUnitFilterWidget() override;

    QString xsdName() const override { return QStringLiteral("FilterOrgUnit"); }

protected:
    QStringList knownKeys() const override;
    void readFromExtras(const QMap<QString, QString> &extras) override;
    QMap<QString, QString> writeToExtras() const override;

private:
    Ui::OrganizationUnitWidget *ui{nullptr};
};

} // namespace preferences

#endif // GPUI_ORGANIZATION_UNIT_FILTER_WIDGET_H
