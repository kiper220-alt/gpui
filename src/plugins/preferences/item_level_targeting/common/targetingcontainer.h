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

#ifndef GPUI_TARGETING_CONTAINER_H
#define GPUI_TARGETING_CONTAINER_H

#include <QList>
#include <QMap>
#include <QMetaType>
#include <QSharedDataPointer>
#include <QString>

namespace preferences
{

//! Normalize a bool attribute value (accepts "AND"/"OR" and legacy "1"/"0").
inline QString combinatorFromString(const QString &value)
{
    if (value == QLatin1String("OR") || value == QLatin1String("0"))
    {
        return QStringLiteral("OR");
    }
    return QStringLiteral("AND");
}

//! One filter entry in a targeting expression tree.
//!
//! Values map 1:1 onto the IFilter base attributes from
//! src/plugins/preferences/schemas/targetingschema.xsd. The skeleton stores
//! a typed name, stable id, combinator/negation, and an attribute map so
//! unknown attributes round-trip verbatim.
struct TargetingFilterRecord
{
    TargetingFilterRecord()
        : name()
        , id()
        , combinator(QStringLiteral("AND"))
        , negated(false)
        , disabled(false)
        , attributes()
        , children()
    {}

    //! XSD element name, e.g. "FilterComputer", "FilterCollection".
    QString name;

    //! Stable GUID from the `id` attribute. Generated for new filters.
    QString id;

    //! Combinator with predecessor. First filter defaults to "AND".
    QString combinator;

    //! Negation (not="1"): apply NOT to this filter's result.
    bool negated;

    //! Whether the filter is currently disabled (MSAD `disabled`).
    bool disabled;

    //! Raw attributes as read from XML. Preserves unknown attributes so
    //! round-trip with MSAD is byte-stable (D7 in design.md).
    QMap<QString, QString> attributes;

    //! Children, used by FilterCollection for parenthesized grouping.
    QList<TargetingFilterRecord> children;
};

class TargetingContainerData;

//! Opaque, copy-on-write container for an item's <Filters> subtree.
//!
//! Held by CommonItem via CommonItem::filters(). The per-snapin
//! reader/writer passes the container through unchanged; only the
//! TargetingDialog inspects contents.
class TargetingContainer
{
public:
    TargetingContainer();
    TargetingContainer(const TargetingContainer &other);
    TargetingContainer(TargetingContainer &&other) noexcept;
    ~TargetingContainer();

    TargetingContainer &operator=(const TargetingContainer &other);
    TargetingContainer &operator=(TargetingContainer &&other) noexcept;

    bool isEmpty() const;

    const QList<TargetingFilterRecord> &filters() const;
    void setFilters(QList<TargetingFilterRecord> filters);

    //! True when any child (at any depth) is a FilterRunOnce entry.
    bool hasRunOnce() const;

private:
    QSharedDataPointer<TargetingContainerData> d;
};

} // namespace preferences

Q_DECLARE_METATYPE(::preferences::TargetingContainer)

#endif // GPUI_TARGETING_CONTAINER_H
