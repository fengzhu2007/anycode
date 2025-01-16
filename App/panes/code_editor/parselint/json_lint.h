#ifndef JSON_LINT_H
#define JSON_LINT_H
#include "../code_lint.h"
#include <textdocument.h>
#include <QString>
#include <QList>

namespace ady{
class JSONLint
{
public:
    JSONLint(TextEditor::TextDocument* textDocument);
    QList<CodeErrorInfo> parse();
private:

    TextEditor::TextDocument* m_textDocument;
    QString m_errorMsg;
    int m_row;
    int m_col;

};
}

#endif // JSON_LINT_H
