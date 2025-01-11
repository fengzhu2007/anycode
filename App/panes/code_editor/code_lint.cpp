#include "code_lint.h"

#include "cmdlint/php_lint.h"
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


CodeErrorInfo CodeLint::checking(const QString& path,const QString& extension){
    QFileInfo fi(path);
    const QString suffix = extension.isEmpty()?fi.suffix().toLower():extension.toLower();
    if(suffix==QLatin1String("php")){
        PHPLint lint(path);
        return lint.parse();
    }
    return {};
}

}
