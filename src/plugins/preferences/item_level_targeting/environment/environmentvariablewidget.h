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

#ifndef GPUI_ENVIRONMENT_VARIABLE_FILTER_WIDGET_H
#define GPUI_ENVIRONMENT_VARIABLE_FILTER_WIDGET_H

#include "targetingfilterwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class EnvironmentWidget; }
QT_END_NAMESPACE

namespace preferences
{

//! Editor for `<FilterVariable variableName=".." value=".."/>`.
class EnvironmentVariableFilterWidget : public TargetingFilterWidget
{
public:
    Q_OBJECT

public:
    explicit EnvironmentVariableFilterWidget(QWidget *parent = nullptr);
    ~EnvironmentVariableFilterWidget() override;

    QString xsdName() const override { return QStringLiteral("FilterVariable"); }

protected:
    QStringList knownKeys() const override;
    void readFromExtras(const QMap<QString, QString> &extras) override;
    QMap<QString, QString> writeToExtras() const override;

private:
    EnvironmentVariableFilterWidget(const EnvironmentVariableFilterWidget &)            = delete;
    EnvironmentVariableFilterWidget(EnvironmentVariableFilterWidget &&)                 = delete;
    EnvironmentVariableFilterWidget &operator=(const EnvironmentVariableFilterWidget &) = delete;
    EnvironmentVariableFilterWidget &operator=(EnvironmentVariableFilterWidget &&)      = delete;

    Ui::EnvironmentWidget *ui{nullptr};
};

} // namespace preferences

#endif // GPUI_ENVIRONMENT_VARIABLE_FILTER_WIDGET_H
