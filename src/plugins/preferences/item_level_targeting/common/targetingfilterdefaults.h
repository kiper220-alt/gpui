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

#ifndef GPUI_TARGETING_FILTER_DEFAULTS_H
#define GPUI_TARGETING_FILTER_DEFAULTS_H

#include <QMap>
#include <QString>

namespace preferences
{

//! Default filter-specific attributes for a newly-created known ILT filter.
QMap<QString, QString> defaultTargetingFilterExtras(const QString &filterName);

//! Merge missing defaults into an existing extras map, preserving loaded
//! unknown attributes and applying per-filter compatibility normalization.
QMap<QString, QString> materializeTargetingFilterExtras(const QString &filterName,
                                                        const QMap<QString, QString> &extras);

} // namespace preferences

#endif // GPUI_TARGETING_FILTER_DEFAULTS_H
