#include "pythonindenter.h"
#include "pythonscanner.h"

#include "tabsettings.h"

#include <algorithm>

namespace Python {

static bool isEmptyLine(const QString &t)
{
    return std::all_of(t.cbegin(), t.cend(), [] (QChar c) { return c.isSpace(); });
}

static inline bool isEmptyLine(const QTextBlock &block)
{
    return isEmptyLine(block.text());
}

static QTextBlock previousNonEmptyBlock(const QTextBlock &block)
{
    QTextBlock result = block;
    while (result.isValid() && isEmptyLine(result))
        result = result.previous();
    return result;
}

class PythonIndenter : public TextEditor::TextIndenter
{
public:
    explicit PythonIndenter(QTextDocument *doc)
        : TextEditor::TextIndenter(doc)
    {}

    QString name() override { return QString::fromUtf8("Python"); }

private:
    bool isElectricCharacter(const QChar &ch) const override;
    int indentFor(const QTextBlock &block,
                  const TextEditor::TabSettings &tabSettings,
                  int cursorPositionInEditor = -1) override;

    bool isElectricLine(const QString &line) const;
    int getIndentDiff(const QString &previousLine,
                      const TextEditor::TabSettings &tabSettings) const;
};

bool PythonIndenter::isElectricCharacter(const QChar &ch) const
{
    return ch == ':';
}

int PythonIndenter::indentFor(const QTextBlock &block,
                              const TextEditor::TabSettings &tabSettings,
                              int /*cursorPositionInEditor*/)
{
    QTextBlock previousBlock = block.previous();
    if (!previousBlock.isValid())
        return 0;

    if (!isEmptyLine(block)) {
        const QTextBlock previousNonEmpty = previousNonEmptyBlock(previousBlock);
        if (previousNonEmpty.isValid())
            previousBlock = previousNonEmpty;
    }

    QString previousLine = previousBlock.text();
    int indentation = tabSettings.indentationColumn(previousLine);

    if (isElectricLine(previousLine))
        indentation += tabSettings.m_indentSize;
    else
        indentation = qMax<int>(0, indentation + getIndentDiff(previousLine, tabSettings));

    return indentation;
}

bool PythonIndenter::isElectricLine(const QString &line) const
{
    if (line.isEmpty())
        return false;

    int index = line.length() - 1;
    while (index > 0 && line[index].isSpace())
        --index;

    return isElectricCharacter(line[index]);
}

int PythonIndenter::getIndentDiff(const QString &previousLine,
                                  const TextEditor::TabSettings &tabSettings) const
{
    static const QStringList jumpKeywords = {
        "return", "yield", "break", "continue", "raise", "pass" };

    Scanner sc(previousLine.constData(), previousLine.length());
    forever {
        auto tk = sc.read();
        if (tk.kind == Token::Keyword && jumpKeywords.contains(sc.value(tk)))
            return -tabSettings.m_indentSize;
        if (tk.kind != Token::Whitespace)
            break;
    }
    return 0;
}

TextEditor::TextIndenter *createIndenter(QTextDocument *doc)
{
    return new PythonIndenter(doc);
}

} // namespace Python
