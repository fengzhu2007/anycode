#include "color_tab.h"
#include "ui_color_tab.h"
#include <texteditorsettings.h>
#include <behaviorsettings.h>
#include <fontsettings.h>
#include <typingsettings.h>
#include <tabsettings.h>
#include <displaysettings.h>
#include <extraencodingsettings.h>
#include "components/list_item_delegate.h"


namespace ady{
class ColorTabPrivate{
public :
    int zoom;
};

ColorTab::ColorTab(QWidget *parent)
    : OptionTab(parent)
    , ui(new Ui::ColorTab)
{
    ui->setupUi(this);

    this->setWindowTitle(tr("Font And Color"));

    connect(ui->font,&QFontComboBox::currentFontChanged,this,&ColorTab::onFontChanged);
    connect(ui->fontSize,QOverload<int>::of(&QSpinBox::valueChanged),this,&ColorTab::onValueChanged);
    connect(ui->zoom,QOverload<int>::of(&QSpinBox::valueChanged),this,&ColorTab::onValueChanged);
    connect(ui->scheme,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&ColorTab::onSchemeChanged);


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

            auto data = TextEditor::FontSettings::schemeMap();
            int i = 0;
            for(auto one:data){
                ui->scheme->addItem(one.first,one.second);
                if(one.second==setting.colorSchemeFileName().toString()){
                    ui->scheme->setCurrentIndex(i);
                }
                ++i;
            }
            ui->scheme->setItemDelegate(new ListItemDelegate(22,ui->scheme));
        }
    }


    ui->editor->setFamily(ui->font->currentFont().family());
    ui->editor->setFontSize(ui->fontSize->value());
    ui->editor->setZoom(ui->zoom->value());
    onSchemeChanged(ui->scheme->currentIndex());


}

void ColorTab::onFontChanged(const QFont& font){
    ui->editor->setFont(font);
    //ui->editor->setFamily(font.family());
}

void ColorTab::onValueChanged(int value){
    auto sender = this->sender();
    if(sender==ui->fontSize){
        ui->editor->setFontSize(value);
        ui->zoom->setValue(100);
    }else if(sender==ui->zoom){
        ui->editor->setFontSize(ui->fontSize->value() * value * 1.0 / 100);
    }
}

void ColorTab::onSchemeChanged(int index){
    auto data = TextEditor::FontSettings::schemeMap();
    QString name  = data.first().first;
    if(index>=0 && index<data.size()){
        name = data.at(index).first;
    }
    ui->editor->setColorScheme(name);
}

}
