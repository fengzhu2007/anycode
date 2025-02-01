#include "language_option_widget.h"
#include "ui_language_option_widget.h"
#include "options_settings.h"
#include "language_settings.h"
#include "language/php_tab.h"
#include <QIcon>

namespace ady{
class LanguageOptionWidgetPrivate{
public:
     QList<OptionTab*> list;
};

LanguageOptionWidget::LanguageOptionWidget(QWidget *parent)
    : OptionWidget(parent)
    , ui(new Ui::LanguageOptionWidget) {

    d = new LanguageOptionWidgetPrivate;
    ui->setupUi(this);

    this->setWindowTitle(tr("Language"));
    this->setWindowIcon(QIcon(":/Resource/icons/Computer_16x.svg"));


    this->initView();
}


LanguageOptionWidget::~LanguageOptionWidget()
{
    delete ui;
}

QString LanguageOptionWidget::name(){
    return LanguageSettings::name();
}

void LanguageOptionWidget::apply(int *state){
    for(auto one:d->list){
        one->apply(state);
    }
}

void LanguageOptionWidget::initValue(const QJsonObject& value){

}


QJsonObject LanguageOptionWidget::toJson() {
    return OptionsSettings::getInstance()->languageSettings().toJson();
}



void LanguageOptionWidget::initView(){
    auto instance = OptionsSettings::getInstance();
    auto setting = instance->languageSettings();
    {
        auto tab = new PHPTab(ui->tabWidget);
        ui->tabWidget->addTab(tab,tab->windowTitle());
        d->list<<tab;
    }
}



}
