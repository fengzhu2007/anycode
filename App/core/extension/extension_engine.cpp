#include "extension_engine.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QPushButton>


namespace ady {
ExtensionEngine* ExtensionEngine::instance = nullptr;
ExtensionEngine::ExtensionEngine(QObject* parent):QJSEngine(parent) {

    installExtensions(QJSEngine::AllExtensions);
    //globalObject().setProperty("QPushButton", newQObject(new QPushButton()));
    //qmlRegisterType();
    //qmlRegisterType<ExtensionEngine>("MyNamespace", 1, 0, "ExtensionEngine");
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
    auto result =  instance->importModule(path);
    if(result.isError()){
        qDebug() << "Error occurred!";
        qDebug() << "Message: " << result.toString();
        qDebug() << "Line number: " << result.property("lineNumber").toInt();
        qDebug() << "Column number: " << result.property("columnNumber").toInt();
        qDebug() << "File name: " << result.property("fileName").toString();
        qDebug() << "Stack trace: " << result.property("stack").toString();

    }else{
        auto func = result.property("sum");
        auto rt = func.call({1,0});
        qDebug()<<rt.toString()<<func.toString();
    }
    auto parent = instance->parent();
    delete instance;
    instance = nullptr;
    init(parent);
    return result;
}



}
