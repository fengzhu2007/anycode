#include "pythonhighlighter.h"
#include "pythonscanner.h"

#include "textdocument.h"
#include "textdocumentlayout.h"
#include "texteditorconstants.h"
#include <utils/qtcassert.h>
#include <QDebug>


namespace Python {

static TextEditor::TextStyle styleForFormat(int kind)
{
    using namespace TextEditor;
    const auto f = Token::Kind(kind);
    switch (f) {
    case Token::Number: return C_NUMBER;
    case Token::String: return C_STRING;
    case Token::Keyword: return C_KEYWORD;
    case Token::Type: return C_TYPE;
    case Token::ClassField: return C_FIELD;
    case Token::MagicAttr: return C_JS_SCOPE_VAR;
    case Token::Operator: return C_OPERATOR;
    case Token::Comment: return C_COMMENT;
    case Token::Doxygen: return C_DOXYGEN_COMMENT;
    case Token::Identifier: return C_TEXT;
    case Token::Whitespace: return C_VISUAL_WHITESPACE;
    case Token::ImportedModule: return C_STRING;
    case Token::Decorator: return C_TYPE;
    case Token::LeftParenthesis: return C_OPERATOR;
    case Token::RightParenthesis: return C_OPERATOR;
    case Token::TokenEnd:
        QTC_CHECK(false);
        return C_TEXT;
    }
    QTC_CHECK(false);
    return C_TEXT;
}

Highlighter::Highlighter()
{
    setDefaultTextFormatCategories();
}

void Highlighter::highlightBlock(const QString &text)
{
    int initialState = previousBlockState();
    if (initialState == -1)
        initialState = 0;
    setCurrentBlockState(highlightLine(text, initialState));
}

static bool isImportKeyword(const QString &keyword)
{
    return keyword == "import" || keyword == "from";
}

static int indent(const QString &line)
{
    for (int i = 0, size = line.size(); i < size; ++i) {
        if (!line.at(i).isSpace())
            return i;
    }
    return -1;
}

static void setFoldingIndent(const QTextBlock &block, int indent)
{
    if (TextEditor::TextBlockUserData *userData = TextEditor::TextDocumentLayout::userData(block)) {
         userData->setFoldingIndent(indent);
         userData->setFoldingStartIncluded(false);
         userData->setFoldingEndIncluded(false);
    }
}

int Highlighter::highlightLine(const QString &text, int initialState)
{
    Scanner scanner(text.constData(), text.size());
    scanner.setState(initialState);

    const int pos = indent(text);
    if (pos < 0) {
        setFoldingIndent(currentBlock(), m_lastIndent);
    } else {
        m_lastIndent = pos;
        if (pos == 0 && text.startsWith('#') && !text.startsWith("#!")) {
            setFoldingIndent(currentBlock(), withinLicenseHeader ? 1 : 0);
            withinLicenseHeader = true;
        } else {
            setFoldingIndent(currentBlock(), m_lastIndent);
            withinLicenseHeader = false;
        }
    }

    Token tk(-1,-1,Token::TokenEnd);
    TextEditor::Parentheses parentheses;
    bool hasOnlyWhitespace = true;
    while (!(tk = scanner.read()).isEndOfBlock()) {
        Token::Kind kind = tk.kind;
        if (kind == Token::Keyword && isImportKeyword(scanner.value(tk)) && hasOnlyWhitespace) {

            setFormat(tk.offset, tk.length, formatForCategory(styleForFormat(kind)));

            highlightImport(scanner);
        } else if (kind == Token::Comment || kind == Token::String || kind == Token::Doxygen) {
            setFormatWithSpaces(text, tk.offset, tk.length, formatForCategory(styleForFormat(kind)));
        } else {
            if (kind == Token::LeftParenthesis) {
                parentheses.append(TextEditor::Parenthesis(TextEditor::Parenthesis::Opened,
                                                           text.at(tk.begin()), tk.begin()));
            } else if (kind == Token::RightParenthesis) {
                parentheses.append(TextEditor::Parenthesis(TextEditor::Parenthesis::Closed,
                                                           text.at(tk.begin()), tk.begin()));
            }
            setFormat(tk.offset, tk.length, formatForCategory(styleForFormat(kind)));
        }

        if (kind != Token::Whitespace)
            hasOnlyWhitespace = false;
    }
    TextEditor::TextDocumentLayout::setParentheses(currentBlock(), parentheses);
    return scanner.state();
}

void Highlighter::highlightImport(Scanner &scanner)
{
    Token tk(-1,-1,Token::TokenEnd);
    while (!(tk = scanner.read()).isEndOfBlock()) {
        Token::Kind kind = tk.kind;
        if (tk.kind == Token::Identifier)
            kind = Token::ImportedModule;
        setFormat(tk.offset, tk.length, formatForCategory(styleForFormat(kind)));
    }
}

} // namespace Python
