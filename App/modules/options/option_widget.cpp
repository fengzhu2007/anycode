#include "option_widget.h"


namespace ady{

OptionWidget::OptionWidget(QWidget* parent):QWidget(parent) {


}


OptionWidget::~OptionWidget(){

}


void OptionWidget::apply(){

}

void OptionWidget::initValue(const QJsonObject& value){
    Q_UNUSED(value);
}

QJsonObject OptionWidget::toJson(){
    return {};
}

}
