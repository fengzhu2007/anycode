#ifndef CODE_LINT_H
#define CODE_LINT_H

#include <QString>
#include <textdocument.h>

namespace ady{

class CodeErrorInfo{
public:
    CodeErrorInfo():level(None),line(0),column(0),length(0){

    };
    CodeErrorInfo(int level,int line,int column,int length,const QString& message):
        level(level),line(line),column(column),length(length),message(message)
    {

    }
    enum Level{
        None=0,
        Warning=1,
        Error
    };
    int level;
    int line;
    int column;
    int length;
    QString message;
};


class CodeLint
{
public:
    CodeLint();
    static void reload(const QString& path);
    static QList<CodeErrorInfo> checking(const QString& path,const QString& extension={});
    static QList<CodeErrorInfo> checking(TextEditor::TextDocument* textDocument,const QString& extension={});

};
}
#endif // CODE_LINT_H
