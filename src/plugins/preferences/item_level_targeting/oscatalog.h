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

#ifndef GPUI_OS_CATALOG_H
#define GPUI_OS_CATALOG_H

#include <QList>
#include <QString>

namespace preferences
{

struct OsChoice
{
    QString label;
    QString value;
};

struct OsProduct
{
    QString label;
    QString value;
    QList<OsChoice> editions;
    QList<OsChoice> servicePacks;
    QList<OsChoice> roles;
    bool visible{true};
};

class OsCatalog
{
public:
    static QList<OsProduct> products();
    static QList<OsProduct> visibleProducts();
    static QList<OsProduct> reservedProducts();
    static OsProduct productByValue(const QString &value);

    static OsChoice anyChoice();
    static OsChoice noServicePacksChoice();
};

} // namespace preferences

#endif // GPUI_OS_CATALOG_H
