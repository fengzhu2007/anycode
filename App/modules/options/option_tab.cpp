#include "option_tab.h"



namespace ady {

OptionTab::OptionTab(QWidget* parent):QWidget(parent) {


}

OptionTab::~OptionTab(){

}

void OptionTab::apply(int *state){

}


void OptionTab::initValue(const QJsonObject& value){
    Q_UNUSED(value);
}

QJsonObject OptionTab::toJson(){
    return {};
}

void OptionTab::notifyChanged(const QString& name,const QVariant& value){
    Q_UNUSED(name);
    Q_UNUSED(value);
}

}
