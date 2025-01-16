#ifndef PYTHON_LINT_H
#define PYTHON_LINT_H
#include "../code_lint.h"
#include <QString>
#include <QString>
namespace ady{
class PythonLint
{
public:
    PythonLint(const QString& path);
    QList<CodeErrorInfo> parse();
private:
    QString m_output;
};
}
#endif // PYTHON_LINT_H
