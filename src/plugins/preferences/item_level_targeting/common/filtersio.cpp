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

#include "filtersio.h"

#include "common/commonitem.h"
#include "common/preferencesmodel.h"
#include "filtersbridge.h"

#include <mvvm/model/sessionitem.h>

#include <QByteArray>
#include <QDomDocument>
#include <QDomElement>
#include <functional>
#include <sstream>

namespace preferences
{

namespace
{

//! Recursive DOM walk. Qt5 `QDomDocument::elementsByTagName("*")` does
//! NOT support wildcards (it looks for the literal tag "*"), so we
//! iterate by hand via `firstChildElement`/`nextSiblingElement`.
template <typename Visitor>
void walkElements(const QDomElement &node, Visitor &&visit)
{
    visit(node);
    for (QDomElement child = node.firstChildElement(); !child.isNull();
         child             = child.nextSiblingElement())
    {
        walkElements(child, visit);
    }
}

//! Same as `walkElements` but takes a snapshot of children before
//! recursing — needed when the visitor mutates the tree (removes
//! children) so iteration stays well-defined.
template <typename Visitor>
void walkElementsMutating(QDomElement node, Visitor &&visit)
{
    visit(node);
    QList<QDomElement> snapshot;
    for (QDomElement child = node.firstChildElement(); !child.isNull();
         child             = child.nextSiblingElement())
    {
        snapshot.append(child);
    }
    for (auto &child : snapshot)
    {
        walkElementsMutating(child, visit);
    }
}

} // namespace

FiltersIO::StripResult FiltersIO::stripFilters(std::istream &input)
{
    StripResult result;

    std::ostringstream buf;
    buf << input.rdbuf();
    const std::string raw = buf.str();
    if (raw.empty())
    {
        return result;
    }

    QDomDocument doc;
    if (!doc.setContent(QByteArray(raw.data(), static_cast<int>(raw.size()))))
    {
        // Fall back to passing the raw XML through unchanged. The
        // sub-plugin's xsd-cxx parser will hit the same parse error
        // and report it; we don't want to mask the failure here.
        result.cleanedXml = raw;
        return result;
    }

    walkElementsMutating(doc.documentElement(), [&](QDomElement node) {
        if (!node.hasAttribute(QStringLiteral("uid")))
        {
            return;
        }
        const auto uid     = node.attribute(QStringLiteral("uid")).toStdString();
        QDomElement filter = node.firstChildElement(QStringLiteral("Filters"));
        if (filter.isNull())
        {
            return;
        }
        // Use the first <Filters> as the canonical source; remove every
        // <Filters> child (xsd-cxx only allows one anyway, but be safe).
        result.filters.emplace(uid, containerFromXml(filter));
        while (!filter.isNull())
        {
            node.removeChild(filter);
            filter = node.firstChildElement(QStringLiteral("Filters"));
        }
    });

    const QByteArray cleaned = doc.toByteArray();
    result.cleanedXml.assign(cleaned.constData(), static_cast<size_t>(cleaned.size()));
    return result;
}

void FiltersIO::applyToModel(PreferencesModel *model, const FiltersMap &filters)
{
    if (!model || filters.empty())
    {
        return;
    }
    for (auto *top : model->rootItem()->children())
    {
        auto *common = dynamic_cast<CommonItem *>(top->getItem("common"));
        if (!common)
        {
            continue;
        }
        const auto uid = common->property<std::string>(
            CommonItem::propertyToString(CommonItem::UID));
        const auto it = filters.find(uid);
        if (it == filters.end())
        {
            continue;
        }
        common->setFiltersFromXml(it->second);
    }
}

std::string FiltersIO::injectFilters(const std::string &xml, const PreferencesModel *model)
{
    if (!model || xml.empty())
    {
        return xml;
    }

    // Index every CommonItem by uid. Sub-plugins register the embedded
    // CommonItem under tag "common" by convention.
    std::map<std::string, CommonItem *> byUid;
    for (auto *top : model->rootItem()->children())
    {
        auto *common = dynamic_cast<CommonItem *>(top->getItem("common"));
        if (!common)
        {
            continue;
        }
        const auto uid = common->property<std::string>(
            CommonItem::propertyToString(CommonItem::UID));
        if (uid.empty())
        {
            continue;
        }
        byUid.emplace(uid, common);
    }
    if (byUid.empty())
    {
        return xml;
    }

    QDomDocument doc;
    if (!doc.setContent(QByteArray(xml.data(), static_cast<int>(xml.size()))))
    {
        return xml;
    }

    bool touched = false;
    walkElementsMutating(doc.documentElement(), [&](QDomElement node) {
        if (!node.hasAttribute(QStringLiteral("uid")))
        {
            return;
        }
        const auto it = byUid.find(node.attribute(QStringLiteral("uid")).toStdString());
        if (it == byUid.end())
        {
            return;
        }

        // Drop any <Filters> xsd-cxx already emitted; we re-emit below
        // from the byte-stable container.
        QDomElement existing = node.firstChildElement(QStringLiteral("Filters"));
        while (!existing.isNull())
        {
            node.removeChild(existing);
            existing = node.firstChildElement(QStringLiteral("Filters"));
        }

        const auto container = it->second->filtersForSerialization();
        if (!container.filters().isEmpty())
        {
            appendContainerToXml(doc, node, container);
            touched = true;
        }

        // Mirror apply-once into the per-extension `image` attribute
        // (D1 in design.md). Only set when it's currently unset or
        // "0" so we don't clobber an icon-index MSAD or another editor
        // has chosen.
        if (it->second->applyOnce())
        {
            const QString current = node.attribute(QStringLiteral("image"));
            if (current.isEmpty() || current == QLatin1String("0"))
            {
                node.setAttribute(QStringLiteral("image"), QStringLiteral("7"));
                touched = true;
            }
        }
    });

    if (!touched)
    {
        return xml;
    }
    const QByteArray updated = doc.toByteArray();
    return std::string(updated.constData(), static_cast<size_t>(updated.size()));
}

} // namespace preferences
