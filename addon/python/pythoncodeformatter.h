#pragma once

#include "codeformatter.h"
#include "python_global.h"

namespace Python {

class PYTHON_EXPORT CodeFormatter : public TextEditor::CodeFormatter
{
public:
    CodeFormatter();
    ~CodeFormatter() override;

    QList<Code::Token> tokenize(const QTextBlock& block) override;
    QList<Code::Token> tokenize(const QString& text) override;
    int indentifierPosition(const QTextBlock& block, int pos) override;
};

} // namespace Python
