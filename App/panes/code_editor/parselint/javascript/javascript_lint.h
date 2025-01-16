#ifndef JAVASCRIPT_LINT_H
#define JAVASCRIPT_LINT_H

#include "../../code_lint.h"
#include <textdocument.h>
#include <QString>
#include <QList>

namespace ady{
class JavascriptLint
{
public:
    JavascriptLint(TextEditor::TextDocument* textDocument);
    QList<CodeErrorInfo> parse();
private:
    TextEditor::TextDocument* m_textDocument;
    QString m_errorMsg;
    QString m_stackInfo;

};
}

#endif // JAVASCRIPT_LINT_H
