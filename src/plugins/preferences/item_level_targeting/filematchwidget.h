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

#ifndef GPUI_FILE_MATCH_FILTER_WIDGET_H
#define GPUI_FILE_MATCH_FILTER_WIDGET_H

#include "targetingfilterwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class FileMatchWidget; }
QT_END_NAMESPACE

namespace preferences
{

//! Editor for `<FilterFile>` (XSD attrs: gte, lte, min, max, path, type,
//! folder).
//!
//! `filematchwidget.ui` uses a `QStackedWidget` to swap between EXISTS and
//! VERSION pages. The first iteration of this widget keeps the stack on
//! its first page and round-trips attributes verbatim — the per-page
//! editing experience is refined in a later iteration.
class FileMatchFilterWidget : public TargetingFilterWidget
{
public:
    Q_OBJECT

public:
    explicit FileMatchFilterWidget(QWidget *parent = nullptr);
    ~FileMatchFilterWidget() override;

    QString xsdName() const override { return QStringLiteral("FilterFile"); }

protected:
    QStringList knownKeys() const override { return {}; }
    void readFromExtras(const QMap<QString, QString> &) override {}
    QMap<QString, QString> writeToExtras() const override { return {}; }

private:
    Ui::FileMatchWidget *ui{nullptr};
};

} // namespace preferences

#endif // GPUI_FILE_MATCH_FILTER_WIDGET_H
