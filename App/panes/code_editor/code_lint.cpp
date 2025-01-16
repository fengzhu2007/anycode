#include "code_lint.h"

#include "cmdlint/php_lint.h"
#include "parselint/python_lint.h"
//#include "parselint/javascript/javascript_lint.h"
#include "parselint/javascript_lint.h"
#include "parselint/json_lint.h"
#include <QLibrary>
#include <QDebug>
#include <QFileInfo>

namespace ady{

CodeLint::CodeLint() {}



void CodeLint::reload(const QString& path){

    /*QLibrary * lib = new QLibrary("E:/php-8.4.2x64/php8");
    bool ret = lib->load();
    qDebug()<<"ret"<<ret;*/

}



QList<CodeErrorInfo> CodeLint::checking(const QString& path,const QString& extension){
    QFileInfo fi(path);
    const QString suffix = extension.isEmpty()?fi.suffix().toLower():extension.toLower();
    if(suffix==QLatin1String("php")){
        PHPLint lint(path);
        return lint.parse();
    }
    return {};
}

QList<CodeErrorInfo> CodeLint::checking(TextEditor::TextDocument* textDocument,const QString& extension){
    auto path = textDocument->filePath().toString();
    QFileInfo fi(path);
    const QString suffix = extension.isEmpty()?fi.suffix().toLower():extension.toLower();
    if(suffix==QLatin1String("php")){
        PHPLint lint(path);
        return lint.parse();
    }else if(suffix==QLatin1String("py")){
        PythonLint lint(textDocument);
        return lint.parse();
    }else if(suffix==QLatin1String("js") || suffix==QLatin1String("jsx")){
        JavascriptLint lint(textDocument);
        return lint.parse();
    }else if(suffix==QLatin1String("json")){
        JSONLint lint(textDocument);
        return lint.parse();
    }
    return {};
}

}
