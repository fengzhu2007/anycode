#include "color_tab.h"
#include "ui_color_tab.h"
#include <texteditorsettings.h>
#include <behaviorsettings.h>
#include <fontsettings.h>
#include <typingsettings.h>
#include <tabsettings.h>
#include <displaysettings.h>
#include <extraencodingsettings.h>


namespace ady{
ColorTab::ColorTab(QWidget *parent)
    : OptionTab(parent)
    , ui(new Ui::ColorTab)
{
    ui->setupUi(this);

    this->setWindowTitle(tr("Font&Color"));

    this->initView();
}

ColorTab::~ColorTab()
{
    delete ui;
}
QString ColorTab::name(){
    return QLatin1String("general");
}

void ColorTab::initValue(const QJsonObject& value){

}

QJsonObject ColorTab::toJson(){

    return {};
}

void ColorTab::initView(){
    auto instance = TextEditor::TextEditorSettings::instance();
    if(instance!=nullptr){
        {
            auto setting = instance->fontSettings();
            ui->font->setCurrentFont(setting.font());
            ui->fontSize->setValue(setting.fontSize());
            ui->zoom->setValue(setting.fontZoom());
        }



    }
}
}
