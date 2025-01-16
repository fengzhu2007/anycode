#ifndef PHP_LINT_H
#define PHP_LINT_H
#include "interface/code_parse_lint.h"
#include <QString>
#include <QString>
namespace ady{
class PHPLint : public CodeParseLint
{
public:
    PHPLint();
    virtual QList<CodeErrorInfo> parse(const QString& source,const QString& path) override;
private:
    void command(const QString& path);
private:
    QString m_output;
};
}
#endif // PHP_LINT_H
