#ifndef JAVASCRIPT_LINT_H
#define JAVASCRIPT_LINT_H
#include "interface/code_parse_lint.h"
#include <textdocument.h>
#include <QString>
#include <QList>

namespace ady{
class JavascriptLint : public CodeParseLint
{
public:
    JavascriptLint();
    virtual QList<CodeErrorInfo> parse(const QString& text,const QString& path) override;

};
}


#endif // JAVASCRIPT_LINT_H
