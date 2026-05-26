/***********************************************************************************************************************
**
** Copyright (C) 2026 BaseALT Ltd. <org@basealt.ru>
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation
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

class RegistryMatchFilterWidget : public TargetingFilterWidget
{
    Q_OBJECT

public:
    explicit RegistryMatchFilterWidget(QWidget *parent = nullptr);
    ~RegistryMatchFilterWidget() override;

    QString xsdName() const override { return QStringLiteral("FilterRegistry"); }

protected:
    QStringList knownKeys() const override;
    void readFromExtras(const QMap<QString, QString> &extras) override;
    QMap<QString, QString> writeToExtras() const override;

private:
    RegistryMatchFilterWidget(const RegistryMatchFilterWidget &)            = delete;
    RegistryMatchFilterWidget(RegistryMatchFilterWidget &&)                 = delete;
    RegistryMatchFilterWidget &operator=(const RegistryMatchFilterWidget &) = delete;
    RegistryMatchFilterWidget &operator=(RegistryMatchFilterWidget &&)      = delete;

    void onMatchTypeChanged(int index);
    void onDataMatchChanged(int index);

    Ui::RegistryMatchWidget *ui{nullptr};
    QString m_version;
};

} // namespace preferences

#endif // GPUI_REGISTRY_MATCH_FILTER_WIDGET_H
