#include "pythonautocompleter.h"

#include "pythonscanner.h"

#include <QTextDocument>
#include <QTextCursor>
#include <QTextBlock>
#include <QDebug>

using namespace Python;

static Code::Token tokenUnderCursor(const QTextCursor &cursor)
{
    const QString blockText = cursor.block().text();
    int prevState = cursor.block().previous().userState();
    if (prevState == -1)
        prevState = 0;

    Scanner scanner(blockText.constData(), blockText.length());
    scanner.setState(prevState);

    const int pos = cursor.positionInBlock();
    Code::Token tk;
    while (!(tk = scanner.read()).isEndOfBlock()) {
        if (tk.kind == Code::Token::Comment || tk.kind == Code::Token::String) {
            if (pos > tk.begin() && pos <= tk.end())
                return tk;
        } else {
            if (pos >= tk.begin() && pos < tk.end())
                return tk;
        }
    }

    return Code::Token();
}

static bool shouldInsertMatchingText(QChar lookAhead)
{
    switch (lookAhead.unicode()) {
    case ')': case ']': case '}':
    case ',': case '\n': case '\r':
        return true;
    default:
        if (lookAhead.isSpace())
            return true;
        return false;
    }
}

static bool shouldInsertMatchingText(const QTextCursor &tc)
{
    QTextDocument *doc = tc.document();
    return shouldInsertMatchingText(doc->characterAt(tc.selectionEnd()));
}

AutoCompleter::AutoCompleter() = default;

AutoCompleter::~AutoCompleter() = default;

bool AutoCompleter::contextAllowsAutoBrackets(const QTextCursor &cursor,
                                              const QString &textToInsert) const
{
    QChar ch;
    if (!textToInsert.isEmpty())
        ch = textToInsert.at(0);

    switch (ch.unicode()) {
    case '(': case '[': case '{':
    case ')': case ']': case '}':
        break;
    default:
        if (ch.isNull())
            break;
        return false;
    }

    const Code::Token token = tokenUnderCursor(cursor);
    switch (token.kind) {
    case Code::Token::Comment:
        return false;
    case Code::Token::String:
        return false;
    default:
        break;
    }

    return true;
}

bool AutoCompleter::contextAllowsAutoQuotes(const QTextCursor &cursor,
                                            const QString &textToInsert) const
{
    if (!isQuote(textToInsert))
        return false;

    const Code::Token token = tokenUnderCursor(cursor);
    switch (token.kind) {
    case Code::Token::Comment:
        return false;
    case Code::Token::String:
        return false;
    default:
        break;
    }

    return true;
}

bool AutoCompleter::contextAllowsElectricCharacters(const QTextCursor &cursor) const
{
    const Code::Token token = tokenUnderCursor(cursor);
    switch (token.kind) {
    case Code::Token::Comment:
    case Code::Token::String:
        return false;
    default:
        return true;
    }
}

bool AutoCompleter::isInComment(const QTextCursor &cursor) const
{
    return tokenUnderCursor(cursor).kind == Code::Token::Comment;
}

QString AutoCompleter::insertMatchingBrace(const QTextCursor &cursor,
                                           const QString &text,
                                           QChar lookAhead,
                                           bool skipChars,
                                           int *skippedChars,int* adjustPos) const
{
    if (text.length() != 1)
        return QString();

    if (!shouldInsertMatchingText(cursor))
        return QString();

    const QChar ch = text.at(0);

    switch (ch.unicode()) {
    case '(':
        return QString(QLatin1Char(')'));
    case '[':
        return QString(QLatin1Char(']'));
    case '{':
        return QString(QLatin1Char('}'));
    case ')':
    case ']':
    case '}':
        if (lookAhead == ch && skipChars)
            ++*skippedChars;
        break;
    default:
        break;
    }

    return QString();
}

QString AutoCompleter::insertMatchingQuote(const QTextCursor &/*tc*/, const QString &text,
                                           QChar lookAhead, bool skipChars, int *skippedChars) const
{
    if (isQuote(text)) {
        if (lookAhead == text && skipChars)
            ++*skippedChars;
        else
            return text;
    }
    return QString();
}

QString AutoCompleter::insertParagraphSeparator(const QTextCursor &cursor) const
{
    return QString();
}
