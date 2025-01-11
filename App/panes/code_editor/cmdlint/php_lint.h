#ifndef PHP_LINT_H
#define PHP_LINT_H
#include "../code_lint.h"
#include <QString>

namespace ady{
class PHPLint
{
public:
    PHPLint(const QString& path);
    CodeErrorInfo parse();
private:
    QString m_output;
};
}
#endif // PHP_LINT_H
