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

#include "targetingfilterwidget.h"

#include "targetingfilteritem.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QRadioButton>
#include <QSpinBox>

namespace preferences
{

TargetingFilterWidget::TargetingFilterWidget(QWidget *parent)
    : BasePreferenceWidget(parent)
    , m_item(nullptr)
    , m_originalExtras()
    , m_liveCommitInstalled(false)
{
}

TargetingFilterWidget::~TargetingFilterWidget() = default;

void TargetingFilterWidget::setItem(ModelView::SessionItem *item)
{
    m_item = dynamic_cast<TargetingFilterItem *>(item);
    if (!m_item)
    {
        m_originalExtras.clear();
        return;
    }

    m_originalExtras = m_item->extras();
    readFromExtras(m_originalExtras);
    installLiveCommitHooks();
}

void TargetingFilterWidget::installLiveCommitHooks()
{
    if (m_liveCommitInstalled)
    {
        return;
    }
    m_liveCommitInstalled = true;

    for (auto *w : findChildren<QLineEdit *>())
    {
        // textChanged fires on every keystroke so the tree row updates
        // live; editingFinished would only fire on focus loss.
        connect(w, &QLineEdit::textChanged, this, &TargetingFilterWidget::submit);
    }
    for (auto *w : findChildren<QCheckBox *>())
    {
        connect(w, &QCheckBox::toggled, this, &TargetingFilterWidget::submit);
    }
    for (auto *w : findChildren<QRadioButton *>())
    {
        connect(w, &QRadioButton::toggled, this, &TargetingFilterWidget::submit);
    }
    for (auto *w : findChildren<QComboBox *>())
    {
        connect(w, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &TargetingFilterWidget::submit);
        // For editable combos, fire on every keystroke too.
        connect(w, &QComboBox::editTextChanged, this, &TargetingFilterWidget::submit);
    }
    for (auto *w : findChildren<QSpinBox *>())
    {
        connect(w, QOverload<int>::of(&QSpinBox::valueChanged),
                this, &TargetingFilterWidget::submit);
    }
    for (auto *w : findChildren<QDoubleSpinBox *>())
    {
        connect(w, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &TargetingFilterWidget::submit);
    }
    for (auto *w : findChildren<QDateTimeEdit *>())
    {
        connect(w, &QDateTimeEdit::dateTimeChanged, this, &TargetingFilterWidget::submit);
    }
    for (auto *w : findChildren<QPlainTextEdit *>())
    {
        connect(w, &QPlainTextEdit::textChanged, this, &TargetingFilterWidget::submit);
    }
}

QString TargetingFilterWidget::name() const
{
    return xsdName();
}

bool TargetingFilterWidget::validate()
{
    return true;
}

void TargetingFilterWidget::submit()
{
    if (!m_item || !validate())
    {
        return;
    }

    QMap<QString, QString> merged = m_originalExtras;
    for (const QString &key : knownKeys())
    {
        merged.remove(key);
    }

    const QMap<QString, QString> known = writeToExtras();
    for (auto it = known.cbegin(); it != known.cend(); ++it)
    {
        merged.insert(it.key(), it.value());
    }

    m_item->setExtras(merged);
    m_item->refreshDisplayName();

    emit dataChanged();
}

} // namespace preferences
