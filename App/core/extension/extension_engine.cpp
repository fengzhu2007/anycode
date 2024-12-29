#include "extension_engine.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QPushButton>
//#include <plugin.h>



namespace ady {
ExtensionEngine* ExtensionEngine::instance = nullptr;
ExtensionEngine::ExtensionEngine(QObject* parent):QJSEngine(parent) {

    installExtensions(QJSEngine::AllExtensions);
    //react_qt::Plugin::install(this);

}

ExtensionEngine::~ExtensionEngine(){

}

ExtensionEngine* ExtensionEngine::getInstance(){

    return instance;
}

void ExtensionEngine::init(QObject* parent){
    if(instance==nullptr){
        instance = new ExtensionEngine(parent);
    }
}

QJSValue ExtensionEngine::run(const QString& path){
    qDebug()<<"run"<<path;
    //auto result = instance->evaluate();
    auto result =  instance->importModule(path);
    if(result.isError()){
        qDebug() << "Error occurred!";
        qDebug() << "Message: " << result.toString();
        qDebug() << "Line number: " << result.property("lineNumber").toInt();
        qDebug() << "Column number: " << result.property("columnNumber").toInt();
        qDebug() << "File name: " << result.property("fileName").toString();
        qDebug() << "Stack trace: " << result.property("stack").toString();

    }
    return result;
}



}
