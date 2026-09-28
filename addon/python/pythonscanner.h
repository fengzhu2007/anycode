#pragma once

#include "languages/token.h"
#include <QString>

namespace Python {
using namespace Code;

class Scanner
{
public:
    Scanner(const Scanner &other) = delete;
    void operator=(const Scanner &other) = delete;

    enum State {
        State_Default,
        State_String,
        State_MultiLineString
    };

    Scanner(const QChar *text, const int length);

    void setState(int state);
    int state() const;
    Token read();
    QString value(const Token& tk) const;

private:
    Token onDefaultState();

    void checkEscapeSequence(QChar quoteChar);
    Token readStringLiteral(QChar quoteChar);
    Token readMultiLineStringLiteral(QChar quoteChar);
    Token readIdentifier();
    Token readNumber();
    Token readFloatNumber();
    Token readComment();
    Token readDoxygenComment();
    Token readWhiteSpace();
    Token readOperator();
    Token readBrace(bool isOpening);
    Token readDecorator();

    bool isAtLineStart() const;
    static bool isStringPrefix(const QString &text, QChar nextChar);

    void clearState();
    void saveState(State state, QChar savedData);
    void parseState(State &state, QChar &savedData) const;

    void setAnchor() { m_markedPosition = m_position; }
    void move() { ++m_position; }
    int length() const { return m_position - m_markedPosition; }
    int anchor() const { return m_markedPosition; }
    bool isEnd() const { return m_position >= m_textLength; }

    QChar peek(int offset = 0) const
    {
        int pos = m_position + offset;
        if (pos >= m_textLength)
            return QLatin1Char('\0');
        return m_text[pos];
    }

    const QChar *m_text;
    const int m_textLength;
    int m_position = 0;
    int m_markedPosition = 0;

    int m_state;
};

} // Python
