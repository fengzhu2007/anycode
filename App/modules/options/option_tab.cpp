#include "option_tab.h"



namespace ady {

OptionTab::OptionTab(QWidget* parent):QWidget(parent) {


}

OptionTab::~OptionTab(){

}


void OptionTab::initValue(const QJsonObject& value){
    Q_UNUSED(value);
}

QJsonObject OptionTab::toJson(){
    return {};
}

}
