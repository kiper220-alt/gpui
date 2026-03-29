#include "systemdunitsyntaxhighlighter.h"

#include <QApplication>
#include <QPalette>
#include <QRegularExpression>

SystemdUnitSyntaxHighlighter::SystemdUnitSyntaxHighlighter(QTextDocument *document)
    : QSyntaxHighlighter(document)
    , m_sectionFormat()
    , m_keyFormat()
    , m_commentFormat()
{
    const auto palette = QApplication::palette();
    m_sectionFormat.setFontWeight(QFont::DemiBold);
    m_sectionFormat.setForeground(palette.brush(QPalette::Link));

    m_keyFormat.setFontWeight(QFont::DemiBold);
    m_keyFormat.setForeground(palette.brush(QPalette::Text));

    const bool isDark = palette.color(QPalette::Window).lightness() < 128;
    m_commentFormat.setForeground(QColor(isDark ? QStringLiteral("#7ca668") : QStringLiteral("#4a7c3f")));
    m_commentFormat.setFontItalic(true);
}

void SystemdUnitSyntaxHighlighter::highlightBlock(const QString &text)
{
    const auto trimmed = text.trimmed();
    if (trimmed.startsWith(QLatin1Char('#')) || trimmed.startsWith(QLatin1Char(';')))
    {
        setFormat(0, text.length(), m_commentFormat);
        return;
    }

    static const QRegularExpression sectionPattern(QStringLiteral("^\\s*\\[[^\\]]+\\]\\s*$"));
    const auto sectionMatch = sectionPattern.match(text);
    if (sectionMatch.hasMatch())
    {
        setFormat(sectionMatch.capturedStart(), sectionMatch.capturedLength(), m_sectionFormat);
        return;
    }

    static const QRegularExpression keyPattern(QStringLiteral("^\\s*([A-Za-z0-9_.\\-]+)\\s*="));
    const auto keyMatch = keyPattern.match(text);
    if (keyMatch.hasMatch())
    {
        setFormat(keyMatch.capturedStart(1), keyMatch.capturedLength(1), m_keyFormat);
    }

    int commentStart = -1;
    bool inSingleQuote = false;
    bool inDoubleQuote = false;
    bool escaped = false;
    for (int i = 0; i < text.size(); ++i)
    {
        const auto ch = text.at(i);
        if (escaped)
        {
            escaped = false;
            continue;
        }

        if (ch == QLatin1Char('\\'))
        {
            escaped = true;
            continue;
        }

        if (!inDoubleQuote && ch == QLatin1Char('\''))
        {
            inSingleQuote = !inSingleQuote;
            continue;
        }

        if (!inSingleQuote && ch == QLatin1Char('"'))
        {
            inDoubleQuote = !inDoubleQuote;
            continue;
        }

        if (!inSingleQuote && !inDoubleQuote
            && (ch == QLatin1Char('#') || ch == QLatin1Char(';'))
            && (i == 0 || text.at(i - 1).isSpace()))
        {
            commentStart = i;
            break;
        }
    }

    if (commentStart >= 0)
    {
        setFormat(commentStart, text.length() - commentStart, m_commentFormat);
    }
}
