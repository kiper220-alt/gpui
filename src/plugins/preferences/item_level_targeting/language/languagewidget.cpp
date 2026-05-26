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

#include "languagewidget.h"
#include "ui_languagewidget.h"

#include <QLocale>
#include <QSet>

namespace preferences
{

namespace
{

QString boolStr(bool checked)
{
    return checked ? QStringLiteral("1") : QStringLiteral("0");
}

bool boolFlag(const QMap<QString, QString> &extras, const QString &key)
{
    return extras.value(key, QStringLiteral("0")) != QLatin1String("0");
}

//! Map QLocale::Language to the MSAD/Win32 Primary Language Identifier
//! (PLID — the low 10 bits of an LCID). The XSD's `language` attribute
//! is the PLID, NOT the full LCID. Values pulled from the official
//! WinNls.h table. Unknown languages return -1 and are skipped from the
//! combo so we don't write a bogus number on save.
//!
//! When new languages are added to QLocale we don't need to grow this
//! table — unmapped entries simply drop out of the combo. Round-tripping
//! a previously-loaded language code works regardless: we display the
//! `displayName` as-is and write the same numeric `language` back.
int qLocaleToPrimaryLanguageId(QLocale::Language lang)
{
    switch (lang)
    {
    case QLocale::Arabic:               return 0x01;
    case QLocale::Bulgarian:            return 0x02;
    case QLocale::Catalan:              return 0x03;
    case QLocale::Chinese:              return 0x04;
    case QLocale::Czech:                return 0x05;
    case QLocale::Danish:               return 0x06;
    case QLocale::German:               return 0x07;
    case QLocale::Greek:                return 0x08;
    case QLocale::English:              return 0x09;
    case QLocale::Spanish:              return 0x0a;
    case QLocale::Finnish:              return 0x0b;
    case QLocale::French:               return 0x0c;
    case QLocale::Hebrew:               return 0x0d;
    case QLocale::Hungarian:            return 0x0e;
    case QLocale::Icelandic:            return 0x0f;
    case QLocale::Italian:              return 0x10;
    case QLocale::Japanese:             return 0x11;
    case QLocale::Korean:               return 0x12;
    case QLocale::Dutch:                return 0x13;
    case QLocale::NorwegianBokmal:      return 0x14;
    case QLocale::Polish:               return 0x15;
    case QLocale::Portuguese:           return 0x16;
    case QLocale::Romansh:              return 0x17;
    case QLocale::Romanian:             return 0x18;
    case QLocale::Russian:              return 0x19;
    case QLocale::Croatian:             return 0x1a;
    case QLocale::Slovak:               return 0x1b;
    case QLocale::Albanian:             return 0x1c;
    case QLocale::Swedish:              return 0x1d;
    case QLocale::Thai:                 return 0x1e;
    case QLocale::Turkish:              return 0x1f;
    case QLocale::Urdu:                 return 0x20;
    case QLocale::Indonesian:           return 0x21;
    case QLocale::Ukrainian:            return 0x22;
    case QLocale::Belarusian:           return 0x23;
    case QLocale::Slovenian:            return 0x24;
    case QLocale::Estonian:             return 0x25;
    case QLocale::Latvian:              return 0x26;
    case QLocale::Lithuanian:           return 0x27;
    case QLocale::Persian:              return 0x29;
    case QLocale::Vietnamese:           return 0x2a;
    case QLocale::Armenian:             return 0x2b;
    case QLocale::Azerbaijani:          return 0x2c;
    case QLocale::Basque:               return 0x2d;
    case QLocale::Macedonian:           return 0x2f;
    case QLocale::Afrikaans:            return 0x36;
    case QLocale::Georgian:             return 0x37;
    case QLocale::Faroese:              return 0x38;
    case QLocale::Hindi:                return 0x39;
    case QLocale::Maltese:              return 0x3a;
    case QLocale::NorthernSami:         return 0x3b;
    case QLocale::Irish:                return 0x3c;
    case QLocale::Malay:                return 0x3e;
    case QLocale::Kazakh:               return 0x3f;
    case QLocale::Kirghiz:              return 0x40;
    case QLocale::Swahili:              return 0x41;
    case QLocale::Uzbek:                return 0x43;
    case QLocale::Tatar:                return 0x44;
    case QLocale::Bengali:              return 0x45;
    case QLocale::Punjabi:              return 0x46;
    case QLocale::Gujarati:             return 0x47;
    case QLocale::Oriya:                return 0x48;
    case QLocale::Tamil:                return 0x49;
    case QLocale::Telugu:               return 0x4a;
    case QLocale::Kannada:              return 0x4b;
    case QLocale::Malayalam:            return 0x4c;
    case QLocale::Assamese:             return 0x4d;
    case QLocale::Marathi:              return 0x4e;
    case QLocale::Sanskrit:             return 0x4f;
    case QLocale::Mongolian:            return 0x50;
    case QLocale::Tibetan:              return 0x51;
    case QLocale::Welsh:                return 0x52;
    case QLocale::Khmer:                return 0x53;
    case QLocale::Lao:                  return 0x54;
    case QLocale::Burmese:              return 0x55;
    case QLocale::Galician:             return 0x56;
    case QLocale::Konkani:              return 0x57;
    case QLocale::Sinhala:              return 0x5b;
    case QLocale::Inuktitut:            return 0x5d;
    case QLocale::Amharic:              return 0x5e;
    case QLocale::Tswana:               return 0x32;
    case QLocale::Xhosa:                return 0x34;
    case QLocale::Zulu:                 return 0x35;
    case QLocale::Yoruba:               return 0x6a;
    case QLocale::Igbo:                 return 0x70;
    default:
        return -1;
    }
}

QString displayNameFor(QLocale::Language lang, QLocale::Country country)
{
    const QString name   = QLocale::languageToString(lang);
    const QString cnName = QLocale::countryToString(country);
    if (cnName.isEmpty() || cnName == QLatin1String("Default"))
    {
        return name;
    }
    return QStringLiteral("%1 (%2)").arg(name, cnName);
}

} // namespace

LanguageFilterWidget::LanguageFilterWidget(QWidget *parent)
    : TargetingFilterWidget(parent)
    , ui(new Ui::LanguageWidget)
    , m_loadedLanguage()
    , m_loadedLocale()
{
    ui->setupUi(this);
    ui->languageComboBox->setEditable(false);

    // Populate the combo with one entry per known QLocale::Language for
    // which we have a Primary Language Id. Sort alphabetically by display
    // name. The combo's user-data is the PLID stored as int.
    QSet<int> seenPlids;
    QList<QPair<QString, int>> entries; // displayName, plid
    const auto allLanguages = QLocale::matchingLocales(
        QLocale::AnyLanguage, QLocale::AnyScript, QLocale::AnyCountry);
    for (const auto &locale : allLanguages)
    {
        const int plid = qLocaleToPrimaryLanguageId(locale.language());
        if (plid < 0 || seenPlids.contains(plid))
        {
            continue;
        }
        seenPlids.insert(plid);
        entries.append({displayNameFor(locale.language(), locale.country()), plid});
    }
    std::sort(entries.begin(), entries.end(),
              [](const QPair<QString, int> &a, const QPair<QString, int> &b) {
                  return a.first.localeAwareCompare(b.first) < 0;
              });
    for (const auto &p : entries)
    {
        ui->languageComboBox->addItem(p.first, p.second);
    }

    // Default to the system locale.
    const int sysPlid = qLocaleToPrimaryLanguageId(QLocale::system().language());
    if (sysPlid >= 0)
    {
        const int idx = ui->languageComboBox->findData(sysPlid);
        if (idx >= 0)
        {
            ui->languageComboBox->setCurrentIndex(idx);
        }
    }
}

LanguageFilterWidget::~LanguageFilterWidget()
{
    delete ui;
}

QStringList LanguageFilterWidget::knownKeys() const
{
    return {QStringLiteral("default"),
            QStringLiteral("system"),
            QStringLiteral("native"),
            QStringLiteral("displayName"),
            QStringLiteral("language"),
            QStringLiteral("locale")};
}

void LanguageFilterWidget::readFromExtras(const QMap<QString, QString> &extras)
{
    m_loadedLanguage = extras.value(QStringLiteral("language"));
    m_loadedLocale = extras.value(QStringLiteral("locale"));

    ui->userCheckBox->setChecked(boolFlag(extras, QStringLiteral("default")));
    ui->systemCheckBox->setChecked(boolFlag(extras, QStringLiteral("system")));
    ui->nativeCheckBox->setChecked(boolFlag(extras, QStringLiteral("native")));

    // language attribute is the Primary Language Id (decimal). Try that
    // first; fall back to matching the displayName text.
    bool ok         = false;
    const int plid  = extras.value(QStringLiteral("language")).toInt(&ok);
    int chosenIndex = -1;
    if (ok && plid > 0)
    {
        chosenIndex = ui->languageComboBox->findData(plid);
    }
    if (chosenIndex < 0)
    {
        const QString displayName = extras.value(QStringLiteral("displayName"));
        if (!displayName.isEmpty())
        {
            chosenIndex = ui->languageComboBox->findText(displayName);
        }
    }
    if (chosenIndex >= 0)
    {
        ui->languageComboBox->setCurrentIndex(chosenIndex);
    }
    else if (ok && plid > 0)
    {
        const QString displayName = extras.value(QStringLiteral("displayName"),
                                                 QString::number(plid));
        ui->languageComboBox->addItem(displayName, plid);
        ui->languageComboBox->setCurrentIndex(ui->languageComboBox->count() - 1);
    }
}

QMap<QString, QString> LanguageFilterWidget::writeToExtras() const
{
    const int idx          = ui->languageComboBox->currentIndex();
    const QString display  = ui->languageComboBox->currentText();
    const QVariant plidVar = (idx >= 0) ? ui->languageComboBox->itemData(idx) : QVariant();
    const QString plid     = plidVar.isValid() ? QString::number(plidVar.toInt()) : QString();
    const QString locale   = (!m_loadedLocale.isEmpty() && plid == m_loadedLanguage)
                               ? m_loadedLocale
                               : plid;

    return {
        {QStringLiteral("default"),     boolStr(ui->userCheckBox->isChecked())},
        {QStringLiteral("system"),      boolStr(ui->systemCheckBox->isChecked())},
        {QStringLiteral("native"),      boolStr(ui->nativeCheckBox->isChecked())},
        {QStringLiteral("displayName"), display},
        {QStringLiteral("language"),    plid},
        {QStringLiteral("locale"),      locale},
    };
}

} // namespace preferences
