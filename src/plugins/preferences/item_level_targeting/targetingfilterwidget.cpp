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

#include "targetingfilterwidget.h"

#include "targetingfilteritem.h"

namespace preferences
{

TargetingFilterWidget::TargetingFilterWidget(QWidget *parent)
    : BasePreferenceWidget(parent)
{
}

TargetingFilterWidget::~TargetingFilterWidget() = default;

void TargetingFilterWidget::setItem(ModelView::SessionItem *item)
{
    m_item = dynamic_cast<TargetingFilterItem *>(item);
    if (!m_item)
    {
        m_originalExtras.clear();
        return;
    }

    m_originalExtras = m_item->extras();
    readFromExtras(m_originalExtras);
}

QString TargetingFilterWidget::name() const
{
    return xsdName();
}

bool TargetingFilterWidget::validate()
{
    return true;
}

void TargetingFilterWidget::submit()
{
    if (!m_item || !validate())
    {
        return;
    }

    QMap<QString, QString> merged = m_originalExtras;
    for (const QString &key : knownKeys())
    {
        merged.remove(key);
    }

    const QMap<QString, QString> known = writeToExtras();
    for (auto it = known.cbegin(); it != known.cend(); ++it)
    {
        merged.insert(it.key(), it.value());
    }

    m_item->setExtras(merged);

    emit dataChanged();
}

} // namespace preferences
