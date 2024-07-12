#include "php_high_lighter.h"
#include <QDebug>
namespace ady{


PHPHighlighter::PHPHighlighter(QTextDocument *parent)
:QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    // Define the format for PHP keywords
    keywordFormat.setForeground(Qt::blue);
    //keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns = {
        "\\babstract\\b", "\\barray\\b", "\\bas\\b", "\\bbreak\\b", "\\bcase\\b", "\\bcatch\\b", "\\bclass\\b", "\\bconst\\b",
        "\\bcontinue\\b", "\\bdeclare\\b", "\\bdefault\\b", "\\bdo\\b", "\\becho\\b", "\\belse\\b", "\\belseif\\b", "\\bempty\\b",
        "\\bextends\\b", "\\bfor\\b", "\\bforeach\\b", "\\bfunction\\b", "\\bglobal\\b", "\\bif\\b", "\\bimplements\\b",
        "\\binclude\\b", "\\binclude_once\\b", "\\binterface\\b", "\\bis\\b", "\\bisset\\b", "\\bnamespace\\b", "\\bnew\\b",
        "\\bprint\\b", "\\bprivate\\b", "\\bprotected\\b", "\\bpublic\\b", "\\brequire\\b", "\\brequire_once\\b", "\\breturn\\b",
        "\\bstatic\\b", "\\bswitch\\b", "\\bthrow\\b", "\\btry\\b", "\\buse\\b", "\\bvar\\b", "\\bwhile\\b", "\\exit\\b","\\b\$_POST\\b","\\b\$_GET\\b","\\b\$_SERVER\\b","\\b<\?php\\b","\\b<\?\\b","\\b\?>\\b"
    };
    for (const QString &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // Define the format for single-line comments
    singleLineCommentFormat.setForeground(Qt::darkGray);
    singleLineCommentFormat.setFontItalic(true);
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    // Define the format for multi-line comments
    multiLineCommentFormat.setForeground(Qt::darkGray);
    multiLineCommentFormat.setFontItalic(true);
    commentStartExpression = QRegularExpression("/\\*");
    commentEndExpression = QRegularExpression("\\*/");

    // Define the format for strings
    quotationFormat.setForeground(Qt::darkRed);
    rule.pattern = QRegularExpression("\".*\"|'.*'");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // Define the format for function names

    //functionFormat.setForeground(Qt::darkMagenta);
    //rule.pattern = QRegularExpression("\\b[A-Za-z0-9_]+(?=\\()");
    //rule.format = functionFormat;
    //highlightingRules.append(rule);
}

void PHPHighlighter::highlightBlock(const QString &text){
    for (const HighlightingRule &rule : qAsConst(highlightingRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    setCurrentBlockState(0);


    // Handle multi-line strings
    //highlightMultilineStrings(text);

    int startIndex = 0;
    if (previousBlockState() != 1) {
        startIndex = text.indexOf(commentStartExpression);
    }

    while (startIndex >= 0) {
        QRegularExpressionMatch endMatch = commentEndExpression.match(text, startIndex);
        int endIndex = endMatch.capturedStart();
        int commentLength = 0;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + endMatch.capturedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
    }


}

void PHPHighlighter::highlightMultilineStrings(const QString &text) {

    if(!text.isEmpty()){
        int startIndex = 0;
        if (previousBlockState() != 2) {
            startIndex = text.indexOf(QRegularExpression("\"|\'"));
        }
        while (startIndex >= 0) {
            QRegularExpression endPattern = QRegularExpression(QString("[^\\\\]") + text.at(startIndex));
            QRegularExpressionMatch endMatch = endPattern.match(text, startIndex + 1);
            int endIndex = endMatch.capturedStart();
            int stringLength;
            if (endIndex == -1) {
                setCurrentBlockState(2);
                stringLength = text.length() - startIndex;
            } else {
                stringLength = endIndex - startIndex + endMatch.capturedLength();
            }
            setFormat(startIndex, stringLength, quotationFormat);
            startIndex = text.indexOf(QRegularExpression("\"|\'"), startIndex + stringLength);
        }
    }
}

}
