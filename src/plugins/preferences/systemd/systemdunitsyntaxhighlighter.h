#ifndef GPUI_SYSTEMD_UNIT_SYNTAX_HIGHLIGHTER_H
#define GPUI_SYSTEMD_UNIT_SYNTAX_HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>

class SystemdUnitSyntaxHighlighter : public QSyntaxHighlighter
{
public:
    explicit SystemdUnitSyntaxHighlighter(QTextDocument *document);

protected:
    void highlightBlock(const QString &text) override;

private:
    QTextCharFormat m_sectionFormat;
    QTextCharFormat m_keyFormat;
    QTextCharFormat m_commentFormat;
};

#endif // GPUI_SYSTEMD_UNIT_SYNTAX_HIGHLIGHTER_H
