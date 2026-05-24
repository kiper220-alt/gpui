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

#include "targetingwriter.h"

#include "targetingfilterdefaults.h"

#include <QDomDocument>
#include <QDomElement>

namespace preferences
{

namespace
{

bool isIdentityAttribute(const QString &name)
{
    return name.compare(QStringLiteral("id"), Qt::CaseInsensitive) == 0
        || name.compare(QStringLiteral("uid"), Qt::CaseInsensitive) == 0;
}

void setGenericAttributes(QDomElement &el, const TargetingFilterRecord &record)
{
    el.setAttribute(QStringLiteral("bool"), combinatorFromString(record.combinator));
    el.setAttribute(QStringLiteral("not"),
                     record.negated ? QStringLiteral("1") : QStringLiteral("0"));
    if (record.disabled)
    {
        el.setAttribute(QStringLiteral("hidden"), QStringLiteral("1"));
    }
    const QMap<QString, QString> attributes =
        materializeTargetingFilterExtras(record.name, record.attributes);
    for (auto it = attributes.cbegin(); it != attributes.cend(); ++it)
    {
        if (isIdentityAttribute(it.key()))
        {
            continue;
        }
        el.setAttribute(it.key(), it.value());
    }
}

} // namespace

bool writeFilterRecord(QDomDocument &doc,
                       QDomElement &parent,
                       const TargetingFilterRecord &record)
{
    if (record.name.isEmpty())
    {
        return false;
    }

    QDomElement el = doc.createElement(record.name);

    if (record.name == QLatin1String("FilterCollection"))
    {
        setGenericAttributes(el, record);
        for (const auto &child : record.children)
        {
            writeFilterRecord(doc, el, child);
        }
    }
    else
    {
        setGenericAttributes(el, record);
    }

    parent.appendChild(el);
    return true;
}

void writeTargetingContainer(QDomDocument &doc,
                             QDomElement &parent,
                             const TargetingContainer &container)
{
    QDomElement filters = doc.createElement(QStringLiteral("Filters"));
    for (const auto &record : container.filters())
    {
        writeFilterRecord(doc, filters, record);
    }
    parent.appendChild(filters);
}

} // namespace preferences
