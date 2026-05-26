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

#include "targetingselection.h"

#include "targetingfilteritem.h"

#include <mvvm/model/sessionitem.h>

#include <QDataStream>
#include <QIODevice>
#include <QMimeData>
#include <QSet>
#include <QVector>

#include <algorithm>
#include <functional>
#include <string>

namespace preferences
{

namespace
{

const std::string kTopLevelFiltersTag = "filters";

QVector<int> treePath(const ModelView::SessionItem *item)
{
    QVector<int> path;
    for (auto *p = item; p && p->parent(); p = p->parent())
    {
        path.prepend(p->tagRow().row);
    }
    return path;
}

bool treeOrderLess(const TargetingFilterItem *a, const TargetingFilterItem *b)
{
    const auto pathA = treePath(a);
    const auto pathB = treePath(b);
    const int limit = std::min(pathA.size(), pathB.size());
    for (int i = 0; i < limit; ++i)
    {
        if (pathA.at(i) != pathB.at(i))
        {
            return pathA.at(i) < pathB.at(i);
        }
    }
    return pathA.size() < pathB.size();
}

} // namespace

const char kTargetingClipboardMimeType[]    = "application/x-gpui-targeting-filter";
const char kTargetingInternalDragMimeType[] = "application/x-gpui-targeting-filter-internal";

bool isTargetingAncestorOf(const ModelView::SessionItem *ancestor,
                           const ModelView::SessionItem *item)
{
    for (auto *p = item; p; p = p->parent())
    {
        if (p == ancestor)
        {
            return true;
        }
    }
    return false;
}

int targetingItemDepth(const ModelView::SessionItem *item)
{
    int depth = 0;
    for (auto *p = item; p; p = p->parent())
    {
        ++depth;
    }
    return depth;
}

QList<TargetingFilterItem *> normalizeTargetingSelection(
    const QList<TargetingFilterItem *> &items)
{
    QList<TargetingFilterItem *> unique;
    QSet<TargetingFilterItem *> seen;
    for (auto *item : items)
    {
        if (!item || !item->parent() || seen.contains(item))
        {
            continue;
        }
        seen.insert(item);
        unique.append(item);
    }

    QList<TargetingFilterItem *> normalized;
    for (auto *item : unique)
    {
        bool coveredByAncestor = false;
        for (auto *other : unique)
        {
            if (item != other && isTargetingAncestorOf(other, item))
            {
                coveredByAncestor = true;
                break;
            }
        }
        if (!coveredByAncestor)
        {
            normalized.append(item);
        }
    }
    return normalized;
}

QList<TargetingFilterItem *> targetingInsertionOrder(
    const QList<TargetingFilterItem *> &items)
{
    auto ordered = normalizeTargetingSelection(items);
    std::sort(ordered.begin(), ordered.end(), treeOrderLess);
    return ordered;
}

QList<TargetingFilterItem *> targetingRemovalOrder(
    const QList<TargetingFilterItem *> &items)
{
    auto ordered = normalizeTargetingSelection(items);
    std::sort(ordered.begin(), ordered.end(),
              [](TargetingFilterItem *a, TargetingFilterItem *b) {
                  const int depthA = targetingItemDepth(a);
                  const int depthB = targetingItemDepth(b);
                  if (depthA != depthB)
                  {
                      return depthA > depthB;
                  }
                  if (a->parent() == b->parent())
                  {
                      return a->tagRow().row > b->tagRow().row;
                  }
                  return a < b;
              });
    return ordered;
}

int adjustedTargetingDropRow(int insertAt,
                             const ModelView::SessionItem *targetParent,
                             const QList<TargetingFilterItem *> &sources)
{
    if (insertAt < 0 || !targetParent)
    {
        return insertAt;
    }

    const int originalInsertAt = insertAt;
    int removedBeforeTarget = 0;
    for (auto *source : normalizeTargetingSelection(sources))
    {
        if (source->parent() == targetParent && source->tagRow().row < originalInsertAt)
        {
            ++removedBeforeTarget;
        }
    }
    return insertAt - removedBeforeTarget;
}

QByteArray encodeTargetingInternalDragData(
    const QString &sourceToken,
    const QList<TargetingFilterItem *> &items)
{
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_12);
    out << sourceToken;
    const auto normalized = targetingInsertionOrder(items);
    out << static_cast<quint32>(normalized.size());
    for (auto *item : normalized)
    {
        out << item->filterId();
    }
    return data;
}

bool decodeTargetingInternalDragData(const QByteArray &data,
                                     const QString &expectedSourceToken,
                                     QStringList &ids)
{
    ids.clear();
    QDataStream in(data);
    in.setVersion(QDataStream::Qt_5_12);

    QString sourceToken;
    quint32 count = 0;
    in >> sourceToken >> count;
    if (in.status() != QDataStream::Ok || sourceToken != expectedSourceToken)
    {
        return false;
    }

    QSet<QString> seenIds;
    for (quint32 i = 0; i < count; ++i)
    {
        QString id;
        in >> id;
        if (in.status() != QDataStream::Ok)
        {
            return false;
        }
        if (!id.isEmpty() && !seenIds.contains(id))
        {
            seenIds.insert(id);
            ids.append(id);
        }
    }
    return !ids.isEmpty();
}

bool resolveTargetingInternalDragData(const QMimeData *mime,
                                      const QString &expectedSourceToken,
                                      ModelView::SessionItem *root,
                                      QList<TargetingFilterItem *> &items)
{
    items.clear();
    if (!mime || !root || !mime->hasFormat(QString::fromLatin1(kTargetingInternalDragMimeType)))
    {
        return false;
    }

    QStringList ids;
    if (!decodeTargetingInternalDragData(
            mime->data(QString::fromLatin1(kTargetingInternalDragMimeType)),
            expectedSourceToken, ids))
    {
        return false;
    }

    for (const auto &id : ids)
    {
        TargetingFilterItem *found = nullptr;
        std::function<void(ModelView::SessionItem *)> walk =
            [&](ModelView::SessionItem *node) {
                if (found)
                {
                    return;
                }
                auto *filter = dynamic_cast<TargetingFilterItem *>(node);
                if (filter && filter->filterId() == id)
                {
                    found = filter;
                    return;
                }
                const auto &tag = filter ? TargetingFilterItem::kChildrenTag
                                         : kTopLevelFiltersTag;
                for (int i = 0; i < node->itemCount(tag); ++i)
                {
                    walk(node->getItem(tag, i));
                }
            };
        walk(root);
        if (!found || !found->parent())
        {
            items.clear();
            return false;
        }
        items.append(found);
    }

    items = normalizeTargetingSelection(items);
    return !items.isEmpty();
}

} // namespace preferences
