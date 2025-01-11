#ifndef CODE_LINT_H
#define CODE_LINT_H

#include <QString>

namespace ady{

class CodeErrorInfo{
public:
    CodeErrorInfo():level(None),number(0),column(0){

    };
    enum Level{
        None=0,
        Warning=1,
        Error
    };
    int level;
    int number;
    int column;
    QString message;
};


class CodeLint
{
public:
    CodeLint();
    static void reload(const QString& path);
    static CodeErrorInfo checking(const QString& path,const QString& extension={});

};
}
#endif // CODE_LINT_H
