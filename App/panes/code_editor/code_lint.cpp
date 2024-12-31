#include "code_lint.h"
#include <QLibrary>
#include <QDebug>


CodeLint::CodeLint() {}



void CodeLint::reload(const QString& path){

    QLibrary * lib = new QLibrary("E:/php-8.4.2x64/php8");
    bool ret = lib->load();
    qDebug()<<"ret"<<ret;

}
