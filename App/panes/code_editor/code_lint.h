#ifndef CODE_LINT_H
#define CODE_LINT_H

#include <QString>
class CodeLint
{
public:
    CodeLint();
    static void reload(const QString& path);
};

#endif // CODE_LINT_H
