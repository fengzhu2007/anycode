#include "pythonscanner.h"

#include <QSet>

namespace Python {

Scanner::Scanner(const QChar *text, const int length)
    : m_text(text), m_textLength(length), m_state(0)
{
}

void Scanner::setState(int state)
{
    m_state = state;
}

int Scanner::state() const
{
    return m_state;
}

Token Scanner::read()
{
    setAnchor();
    if (isEnd())
        return Token(-1,-1,Token::TokenEnd);

    State state;
    QChar saved;
    parseState(state, saved);
    switch (state) {
    case State_String:
        return readStringLiteral(saved);
    case State_MultiLineString:
        return readMultiLineStringLiteral(saved);
    default:
        return onDefaultState();
    }
}

QString Scanner::value(const Token &tk) const
{
    return QString(m_text + tk.offset, tk.length);
}

bool Scanner::isAtLineStart() const
{
    for (int i = m_markedPosition - 1; i >= 0; --i) {
        if (m_text[i] == '\n')
            return true;
        if (!m_text[i].isSpace())
            return false;
    }
    return true;
}

bool Scanner::isStringPrefix(const QString &text, QChar nextChar)
{
    if (nextChar != '\'' && nextChar != '"')
        return false;

    static const QSet<QString> prefixes = {
        "f", "F", "r", "R", "b", "B",
        "rf", "rF", "Rf", "RF", "fr", "fR", "Fr", "FR",
        "rb", "rB", "Rb", "RB", "br", "bR", "Br", "BR",
        "bf", "bF", "Bf", "BF", "fb", "fB", "Fb", "FB",
        "ur", "uR", "Ur", "UR", "u", "U",
    };
    return prefixes.contains(text);
}

Token Scanner::onDefaultState()
{
    QChar first = peek();
    move();

    if (first == '\\' && peek() == '\n') {
        move();
        return Token(anchor(), 2,Token::Whitespace);
    }

    if (first == '.' && peek().isDigit())
        return readFloatNumber();

    if (first == '\'' || first == '\"')
        return readStringLiteral(first);

    if (first.isLetter() || first == '_')
        return readIdentifier();

    if (first.isDigit())
        return readNumber();

    if (first == '#') {
        if (peek() == '#')
            return readDoxygenComment();
        return readComment();
    }

    if (first == '@' && isAtLineStart())
        return readDecorator();

    if (first == '(' || first == '[' || first == '{')
        return readBrace(true);
    if (first == ')' || first == ']' || first == '}')
        return readBrace(false);

    if (first.isSpace())
        return readWhiteSpace();

    return readOperator();
}

void Scanner::checkEscapeSequence(QChar quoteChar)
{
    if (peek() == '\\') {
        move();
        QChar ch = peek();
        if (ch == '\n' || ch.isNull())
            saveState(State_String, quoteChar);
    }
}

Token Scanner::readStringLiteral(QChar quoteChar)
{
    QChar ch = peek();
    if (ch == quoteChar && peek(1) == quoteChar) {
        saveState(State_MultiLineString, quoteChar);
        return readMultiLineStringLiteral(quoteChar);
    }

    while (ch != quoteChar && !ch.isNull()) {
        checkEscapeSequence(quoteChar);
        move();
        ch = peek();
    }
    if (ch == quoteChar)
        clearState();
    move();
    return Token(anchor(), length(),Token::String);
}

Token Scanner::readMultiLineStringLiteral(QChar quoteChar)
{
    for (;;) {
        QChar ch = peek();
        if (ch.isNull())
            break;
        if (ch == quoteChar && peek(1) == quoteChar && peek(2) == quoteChar) {
            clearState();
            move();
            move();
            move();
            break;
        }
        move();
    }
    return Token(anchor(), length(),Token::String);
}

Token Scanner::readIdentifier()
{
    // Fix 1: Updated keyword list for Python 3
    static const QSet<QString> keywords = {
        "and", "as", "assert", "async", "await",
        "break", "class", "continue", "def", "del",
        "elif", "else", "except", "finally", "for",
        "from", "global", "if", "import", "in", "is",
        "lambda", "match", "case", "type",
        "not", "or", "pass", "raise", "return", "try",
        "while", "with", "yield"
    };

    static const QSet<QString> magics = {
        "__init__", "__del__", "__new__",
        "__str__", "__repr__", "__format__",
        "__setattr__", "__getattr__", "__delattr__", "__getattribute__",
        "__add__", "__sub__", "__mul__", "__truediv__", "__floordiv__", "__mod__",
        "__pow__", "__and__", "__or__", "__xor__",
        "__eq__", "__ne__", "__gt__", "__lt__", "__ge__", "__le__",
        "__lshift__", "__rshift__", "__contains__",
        "__pos__", "__neg__", "__inv__", "__abs__", "__len__", "__bool__",
        "__getitem__", "__setitem__", "__delitem__",
        "__iter__", "__next__", "__reversed__",
        "__call__", "__enter__", "__exit__",
        "__hash__", "__index__", "__round__", "__trunc__", "__floor__", "__ceil__",
        "__copy__", "__deepcopy__", "__sizeof__",
        "__class__", "__name__", "__module__", "__dict__", "__bases__",
        "__doc__", "__qualname__", "__slots__", "__annotations__",
        "__init_subclass__", "__set_name__", "__missing__",
        "__instancecheck__", "__subclasscheck__",
        "__await__", "__aiter__", "__anext__", "__aenter__", "__aexit__",
    };

    // Fix 2: Complete Python 3 builtins list
    static const QSet<QString> builtins = {
        // Types
        "int", "float", "complex", "str", "bytes", "bytearray",
        "list", "tuple", "dict", "set", "frozenset",
        "bool", "type", "object", "range",
        // Constants
        "None", "True", "False", "NotImplemented", "Ellipsis", "__debug__",
        // Functions
        "abs", "all", "any", "ascii", "bin", "callable", "chr", "classmethod",
        "compile", "delattr", "dir", "divmod", "enumerate", "eval", "exec",
        "filter", "format", "getattr", "globals", "hasattr", "hash", "help",
        "hex", "id", "input", "isinstance", "issubclass", "iter", "len",
        "locals", "map", "max", "memoryview", "min", "next", "oct", "open",
        "ord", "pow", "print", "property", "repr", "reversed", "round",
        "setattr", "slice", "sorted", "staticmethod", "sum", "super",
        "vars", "zip", "__import__",
        // Common exceptions
        "BaseException", "Exception", "ArithmeticError", "AssertionError",
        "AttributeError", "BlockingIOError", "BrokenPipeError",
        "BufferError", "BytesWarning", "ChildProcessError",
        "ConnectionAbortedError", "ConnectionError",
        "ConnectionRefusedError", "ConnectionResetError",
        "DeprecationWarning", "EOFError", "EnvironmentError",
        "FileExistsError", "FileNotFoundError", "FloatingPointError",
        "FutureWarning", "GeneratorExit", "IOError", "ImportError",
        "ImportWarning", "IndexError", "InterruptedError",
        "IsADirectoryError", "KeyError", "KeyboardInterrupt",
        "LookupError", "MemoryError", "ModuleNotFoundError",
        "NameError", "NotADirectoryError", "NotImplementedError",
        "OSError", "OverflowError", "PendingDeprecationWarning",
        "PermissionError", "ProcessLookupError", "RecursionError",
        "ReferenceError", "ResourceWarning", "RuntimeError",
        "RuntimeWarning", "StopAsyncIteration", "StopIteration",
        "SyntaxError", "SyntaxWarning", "SystemError", "SystemExit",
        "TabError", "TimeoutError", "TypeError", "UnboundLocalError",
        "UnicodeDecodeError", "UnicodeEncodeError", "UnicodeError",
        "UnicodeTranslationError", "UnicodeWarning", "UserWarning",
        "ValueError", "Warning", "ZeroDivisionError",
    };

    QChar ch = peek();
    while (ch.isLetterOrNumber() || ch == '_') {
        move();
        ch = peek();
    }

    const QString v = QString(m_text + m_markedPosition, length());

    // Fix 3: Check for string prefix (f"...", r"...", b"...", etc.)
    if (isStringPrefix(v, peek())) {
        QChar quoteChar = peek();
        move(); // consume the quote
        return readStringLiteral(quoteChar);
    }

    Token::Kind kind = Token::Identifier;
    if (v == "self" || v == "cls")
        kind = Token::ClassField;
    else if (builtins.contains(v))
        kind = Token::Type;
    else if (magics.contains(v))
        kind = Token::MagicAttr;
    else if (keywords.contains(v))
        kind = Token::Keyword;

    return Token(anchor(), length(),kind);
}

inline static bool isHexDigit(QChar ch)
{
    return ch.isDigit()
            || (ch >= 'a' && ch <= 'f')
            || (ch >= 'A' && ch <= 'F');
}

inline static bool isOctalDigit(QChar ch)
{
    return ch.isDigit() && ch != '8' && ch != '9';
}

inline static bool isBinaryDigit(QChar ch)
{
    return ch == '0' || ch == '1';
}

inline static bool isValidIntegerSuffix(QChar ch)
{
    return ch == 'l' || ch == 'L';
}

Token Scanner::readNumber()
{
    if (!isEnd()) {
        QChar ch = peek();
        if (ch.toLower() == 'b') {
            move();
            while (isBinaryDigit(peek()))
                move();
        } else if (ch.toLower() == 'o') {
            move();
            while (isOctalDigit(peek()))
                move();
        } else if (ch.toLower() == 'x') {
            move();
            while (isHexDigit(peek()))
                move();
        } else {
            return readFloatNumber();
        }
        if (isValidIntegerSuffix(peek()))
            move();
    }
    return Token(anchor(), length(),Token::Number);
}

Token Scanner::readFloatNumber()
{
    enum
    {
        State_INTEGER,
        State_FRACTION,
        State_EXPONENT
    } state;
    state = (peek(-1) == '.') ? State_FRACTION : State_INTEGER;

    for (;;) {
        QChar ch = peek();
        if (ch.isNull())
            break;

        if (state == State_INTEGER) {
            if (ch == '.')
                state = State_FRACTION;
            else if (!ch.isDigit())
                break;
        } else if (state == State_FRACTION) {
            if (ch == 'e' || ch == 'E') {
                QChar next = peek(1);
                QChar next2 = peek(2);
                bool isExp = next.isDigit()
                        || ((next == '-' || next == '+') && next2.isDigit());
                if (isExp) {
                    move();
                    state = State_EXPONENT;
                } else {
                    break;
                }
            } else if (!ch.isDigit()) {
                break;
            }
        } else if (!ch.isDigit()) {
            break;
        }
        move();
    }

    QChar ch = peek();
    if ((state == State_INTEGER && (ch == 'l' || ch == 'L'))
            || (ch == 'j' || ch =='J'))
        move();

    return Token(anchor(), length(),Token::Number);
}

Token Scanner::readComment()
{
    QChar ch = peek();
    while (ch != '\n' && !ch.isNull()) {
        move();
        ch = peek();
    }
    return Token(anchor(), length(),Token::Comment);
}

Token Scanner::readDoxygenComment()
{
    QChar ch = peek();
    while (ch != '\n' && !ch.isNull()) {
        move();
        ch = peek();
    }
    return Token(anchor(), length(),Token::Doxygen);
}

Token Scanner::readWhiteSpace()
{
    while (peek().isSpace())
        move();
    return Token(anchor(), length(),Token::Whitespace);
}

Token Scanner::readOperator()
{
    // Fix 4: Exclude '@' from operator chars so it's handled as decorator
    static const QString EXCLUDED_CHARS = "\'\"_#([{}])@";
    QChar ch = peek();
    while (ch.isPunct() && !EXCLUDED_CHARS.contains(ch)) {
        move();
        ch = peek();
    }
    return Token(anchor(), length(),Token::Operator);
}

Token Scanner::readBrace(bool isOpening)
{
    Token::Kind kind = isOpening ? Token::LeftParenthesis : Token::RightParenthesis;
    return Token(anchor(), length(),kind);
}

// Fix 4: Read @identifier as a single Decorator token
Token Scanner::readDecorator()
{
    QChar ch = peek();
    while (ch.isSpace() && ch != '\n') {
        move();
        ch = peek();
    }
    while (ch.isLetterOrNumber() || ch == '_' || ch == '.') {
        move();
        ch = peek();
    }
    return Token(anchor(), length(), Token::Decorator);
}

void Scanner::clearState()
{
    m_state = 0;
}

void Scanner::saveState(State state, QChar savedData)
{
    m_state = (state << 16) | static_cast<int>(savedData.unicode());
}

void Scanner::parseState(State &state, QChar &savedData) const
{
    state = static_cast<State>(m_state >> 16);
    savedData = static_cast<ushort>(m_state);
}

} // Python
