#include "code_lint.h"

#include "cmdlint/php_lint.h"
#include "parselint/python_lint.h"
#include "parselint/typescript_lint.h"
#include "parselint/javascript_lint.h"
#include "parselint/json_lint.h"
#include "parselint/html_lint.h"
#include "parselint/css_lint.h"
#include "parselint/xml_lint.h"
#include "parselint/cpp_lint.h"
#include <QLibrary>
#include <QDebug>
#include <QFileInfo>
#include <QTextDocument>

namespace ady{

CodeLint* CodeLint::instance = nullptr;


CodeLint::CodeLint() {

}



void CodeLint::init(){
    if(!instance){
        instance = new CodeLint();
        instance->registerParser<PythonLint>("py");
        instance->registerParser<PHPLint>("php");
        instance->registerParser<JavascriptLint>("js");
        instance->registerParser<JavascriptLint>("jsx");
        instance->registerParser<JSONLint>("json");
        instance->registerParser<TypescriptLint>("ts");
        instance->registerParser<TypescriptLint>("tsx");
        instance->registerParser<HTMLLint>("html");
        instance->registerParser<CSSLint>("css");
        instance->registerParser<CPPLint>("cpp");
        instance->registerParser<XMLLint>("xml");
    }
}

void CodeLint::destory(){
    if(instance){
        delete instance;
        instance = nullptr;
    }
}

CodeLint* CodeLint::getInstance(){
    return instance;
}



QList<CodeErrorInfo> CodeLint::checking(const QString& path,const QString& extension){

    return {};
}

QList<CodeErrorInfo> CodeLint::checking(TextEditor::TextDocument* textDocument,const QString& extension){
    auto path = textDocument->filePath().toString();
    auto source = textDocument->document()->toPlainText();
    QFileInfo fi(path);
    CodeParseLint* lint = nullptr;
    const QString suffix = extension.isEmpty()?fi.suffix().toLower():extension.toLower();
    if(instance){
        auto iter = instance->m_parserlist.find(suffix);
        if(iter!=instance->m_parserlist.end()){
            lint = iter->second();
            lint->parse(source,path);
            auto list = lint->results();
            delete lint;
            return list;
        }
    }
    return {};


}

}
