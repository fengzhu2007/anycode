#include "pythoncodeformatter.h"
#include "pythonscanner.h"

#include <QTextBlock>

namespace Python {

CodeFormatter::CodeFormatter() = default;

CodeFormatter::~CodeFormatter() = default;

QList<Code::Token> CodeFormatter::tokenize(const QString& text)
{
    QList<Code::Token> tokens;
    Scanner scanner(text.constData(), text.length());
    Code::Token tk;
    while (!(tk = scanner.read()).isEndOfBlock()) {
        tokens.append(tk);
    }
    return tokens;
}

QList<Code::Token> CodeFormatter::tokenize(const QTextBlock& block)
{
    return tokenize(block.text());
}

int CodeFormatter::indentifierPosition(const QTextBlock& block, int pos)
{
    int blockPos = pos - block.position();
    const QString text = block.text();
    if (blockPos < 1)
        return pos;
    if (blockPos > text.length())
        blockPos = text.length();
    do {
        if (!isIdentifier(text.at(--blockPos))) {
            ++blockPos;
            break;
        }
    } while (blockPos > 0);
    return block.position() + blockPos;
}

} // namespace Python
