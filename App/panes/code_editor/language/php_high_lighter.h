#ifndef PHPHIGHLIGHTER_H
#define PHPHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QColor>
#include <QBrush>

namespace ady{
    class PHPHighlighter : public QSyntaxHighlighter
    {
        Q_OBJECT
    public:
        PHPHighlighter(QTextDocument *parent = nullptr);

    protected:
        void highlightBlock(const QString &text) override;

    private:
        void highlightMultilineStrings(const QString &text);

    private:
        struct HighlightingRule {
            QRegularExpression pattern;
            QTextCharFormat format;
        };
        QVector<HighlightingRule> highlightingRules;

        QRegularExpression commentStartExpression;
        QRegularExpression commentEndExpression;

        QTextCharFormat keywordFormat;
        QTextCharFormat singleLineCommentFormat;
        QTextCharFormat multiLineCommentFormat;
        QTextCharFormat quotationFormat;
        QTextCharFormat functionFormat;
    };
}

#endif // PHPHIGHLIGHTER_H
