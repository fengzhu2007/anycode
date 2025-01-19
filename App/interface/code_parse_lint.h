#ifndef CODE_PARSE_LINT_H
#define CODE_PARSE_LINT_H
#include "global.h"
#include <QString>
#include <QList>


struct TSLanguage;
struct TSNode;
namespace ady {

class ANYENGINE_EXPORT CodeErrorInfo{
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


class CodeParseLintPrivate;
class ANYENGINE_EXPORT CodeParseLint
{
public:
    CodeParseLint();
    virtual ~CodeParseLint();
    virtual void parse(const QString& source,const QString& path);
    virtual QList<CodeErrorInfo> results();

protected:
    void setup(const TSLanguage* l);
    int row();
    int col();
    int length();
    QString& message();
    void setRow(int row);
    void setCol(int col);
    void setLength(int length);
    void setMessage(const QString& message);
    void walkErrors(const TSNode& node);

private:
    CodeParseLintPrivate* d;
};

}

#endif // CODE_PARSE_LINT_H
