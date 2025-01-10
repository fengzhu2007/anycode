#include "option_widget.h"


namespace ady{

OptionWidget::OptionWidget(QWidget* parent):QWidget(parent) {


}


OptionWidget::~OptionWidget(){

}


void OptionWidget::apply(int *state){

}

void OptionWidget::initValue(const QJsonObject& value){
    Q_UNUSED(value);
}

QJsonObject OptionWidget::toJson(){
    return {};
}

void OptionWidget::notifyChanged(const QString& name,const QVariant& value){
    Q_UNUSED(name);
    Q_UNUSED(value);
}

}
