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

#ifndef GPUI_TARGETING_SELECTION_H
#define GPUI_TARGETING_SELECTION_H

#include <QByteArray>
#include <QList>
#include <QString>
#include <QStringList>

QT_BEGIN_NAMESPACE
class QMimeData;
QT_END_NAMESPACE

namespace ModelView
{
class SessionItem;
}

namespace preferences
{

class TargetingFilterItem;

extern const char kTargetingClipboardMimeType[];
extern const char kTargetingInternalDragMimeType[];

bool isTargetingAncestorOf(const ModelView::SessionItem *ancestor,
                           const ModelView::SessionItem *item);
int targetingItemDepth(const ModelView::SessionItem *item);

QList<TargetingFilterItem *> normalizeTargetingSelection(
    const QList<TargetingFilterItem *> &items);
QList<TargetingFilterItem *> targetingInsertionOrder(
    const QList<TargetingFilterItem *> &items);
QList<TargetingFilterItem *> targetingRemovalOrder(
    const QList<TargetingFilterItem *> &items);
int adjustedTargetingDropRow(int insertAt,
                             const ModelView::SessionItem *targetParent,
                             const QList<TargetingFilterItem *> &sources);

QByteArray encodeTargetingInternalDragData(
    const QString &sourceToken,
    const QList<TargetingFilterItem *> &items);
bool decodeTargetingInternalDragData(const QByteArray &data,
                                     const QString &expectedSourceToken,
                                     QStringList &ids);
bool resolveTargetingInternalDragData(const QMimeData *mime,
                                      const QString &expectedSourceToken,
                                      ModelView::SessionItem *root,
                                      QList<TargetingFilterItem *> &items);

} // namespace preferences

#endif // GPUI_TARGETING_SELECTION_H
