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

#include "targetingreader.h"

#include <QDomElement>
#include <QDomNamedNodeMap>

namespace preferences
{

namespace
{

TargetingFilterRecord recordFromElement(const QDomElement &element)
{
    TargetingFilterRecord record;
    record.name = element.tagName();

    const QDomNamedNodeMap attrs = element.attributes();
    for (int i = 0; i < attrs.count(); ++i)
    {
        const QDomAttr attr = attrs.item(i).toAttr();
        if (attr.isNull())
        {
            continue;
        }

        const QString name  = attr.name();
        const QString value = attr.value();

        if (name == QLatin1String("bool"))
        {
            record.combinator = combinatorFromString(value);
        }
        else if (name == QLatin1String("not"))
        {
            record.negated = (value == QLatin1String("1"));
        }
        else if (name == QLatin1String("hidden"))
        {
            record.disabled = (value == QLatin1String("1"));
        }
        else if (name == QLatin1String("id"))
        {
            record.id = value;
        }
        else
        {
            record.attributes.insert(name, value);
        }
    }

    if (record.name == QLatin1String("FilterCollection"))
    {
        for (QDomElement child = element.firstChildElement(); !child.isNull();
             child             = child.nextSiblingElement())
        {
            record.children.append(recordFromElement(child));
        }
    }

    return record;
}

} // namespace

TargetingContainer readTargetingContainer(const QDomElement &filtersElement)
{
    QList<TargetingFilterRecord> records;
    for (QDomElement child = filtersElement.firstChildElement(); !child.isNull();
         child             = child.nextSiblingElement())
    {
        records.append(recordFromElement(child));
    }

    TargetingContainer out;
    out.setFilters(std::move(records));
    return out;
}

} // namespace preferences
