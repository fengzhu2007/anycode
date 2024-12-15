#include "extension_engine.h"
#include <QDebug>


namespace ady {
ExtensionEngine* ExtensionEngine::instance = nullptr;
ExtensionEngine::ExtensionEngine(QObject* parent):QJSEngine(parent) {



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
        func.call({1,0});

    }
    return result;
}



}
