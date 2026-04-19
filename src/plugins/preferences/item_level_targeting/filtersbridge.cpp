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

#include "filtersbridge.h"

#include <QDomDocument>
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
            // MSAD: bool="1" → AND, bool="0" → OR.
            record.combinator = (value == QLatin1String("0"))
                                    ? QStringLiteral("OR")
                                    : QStringLiteral("AND");
        }
        else if (name == QLatin1String("not"))
        {
            record.negated = (value == QLatin1String("1"));
        }
        else if (name == QLatin1String("hidden"))
        {
            // MSAD's IFilter@hidden carries the "disabled" flag.
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

void writeRecord(QDomDocument &doc, QDomElement &parent, const TargetingFilterRecord &record)
{
    if (record.name.isEmpty())
    {
        return;
    }
    QDomElement element = doc.createElement(record.name);

    element.setAttribute(QStringLiteral("bool"),
                         record.combinator == QLatin1String("OR")
                             ? QStringLiteral("0")
                             : QStringLiteral("1"));
    element.setAttribute(QStringLiteral("not"),
                         record.negated ? QStringLiteral("1") : QStringLiteral("0"));
    if (record.disabled)
    {
        element.setAttribute(QStringLiteral("hidden"), QStringLiteral("1"));
    }
    if (!record.id.isEmpty())
    {
        element.setAttribute(QStringLiteral("id"), record.id);
    }
    for (auto it = record.attributes.cbegin(); it != record.attributes.cend(); ++it)
    {
        element.setAttribute(it.key(), it.value());
    }

    if (record.name == QLatin1String("FilterCollection"))
    {
        for (const auto &child : record.children)
        {
            writeRecord(doc, element, child);
        }
    }

    parent.appendChild(element);
}

} // namespace

TargetingContainer containerFromXml(const QDomElement &filtersElement)
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

void appendContainerToXml(QDomDocument &doc,
                          QDomElement &parent,
                          const TargetingContainer &container)
{
    QDomElement filters = doc.createElement(QStringLiteral("Filters"));
    for (const auto &record : container.filters())
    {
        writeRecord(doc, filters, record);
    }
    parent.appendChild(filters);
}

} // namespace preferences
