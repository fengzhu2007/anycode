#ifndef CODE_LINT_H
#define CODE_LINT_H


#include "interface/code_parse_lint.h"
#include "global.h"
#include <map>
#include <QString>
#include <textdocument.h>

namespace ady{

using FactoryFunc = std::function<CodeParseLint*()>;


class ANYENGINE_EXPORT CodeLint
{
public:
    template <typename T>
    void registerParser(const QString& name){
        m_parserlist[name] = [](){
            return new T;
        };
    }
    static void init();
    static void destory();
    static CodeLint* getInstance();
    static QList<CodeErrorInfo> checking(const QString& path,const QString& extension={});
    static QList<CodeErrorInfo> checking(TextEditor::TextDocument* textDocument,const QString& extension={});
    static void settingsChanged();

private:
    CodeLint();

private:
    static CodeLint* instance;
    std::map<QString,FactoryFunc> m_parserlist;


};
}
#endif // CODE_LINT_H
