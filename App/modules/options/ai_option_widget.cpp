#include "ai_option_widget.h"
#include "ui_ai_option_widget.h"
#include "options_settings.h"
#include "ai_settings.h"
#include <QIcon>


namespace ady{


AIOptionWidget::AIOptionWidget(QWidget *parent)
    : OptionWidget(parent)
    , ui(new Ui::AIOptionWidget)
{
    ui->setupUi(this);

    this->setWindowTitle(tr("AI Assistant"));
    this->setWindowIcon(QIcon(":/Resource/icons/ai.svg"));

    this->initView();
}

AIOptionWidget::~AIOptionWidget()
{
    delete ui;
}


QString AIOptionWidget::name(){
    return AISettings::name();
}

void AIOptionWidget::apply(int *state){

}

void AIOptionWidget::initValue(const QJsonObject& value){

}

QJsonObject AIOptionWidget::toJson() {
    return OptionsSettings::getInstance()->aiSettings().toJson();
}

void AIOptionWidget::initView(){

}

}
