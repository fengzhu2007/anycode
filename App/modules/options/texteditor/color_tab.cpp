#include "color_tab.h"
#include "ui_color_tab.h"
#include <texteditorsettings.h>
#include <behaviorsettings.h>
#include <fontsettings.h>
#include <typingsettings.h>
#include <tabsettings.h>
#include <displaysettings.h>
#include <extraencodingsettings.h>
#include <fontsettings.h>
#include "components/list_item_delegate.h"
#include "../option_widget.h"
#include "../options_settings.h"
#include "../environment_settings.h"


namespace ady{

const QString ColorTab::themeNameKey = "texteditor::theme";

class ColorTabPrivate{
public :
    int zoom;
    QList<QPair<QString,QString>> themeData;
};

ColorTab::ColorTab(QWidget *parent)
    : OptionTab(parent)
    , ui(new Ui::ColorTab)
{
    d = new ColorTabPrivate;
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
    delete d;
}
QString ColorTab::name(){
    return QLatin1String("general");
}

void ColorTab::apply(int *state){
    auto instance = TextEditor::TextEditorSettings::instance();
    if(instance!=nullptr){
        {
            bool changed = false;
            auto setting = instance->fontSettings();

            const QString family = ui->font->currentFont().family();
            if(family!=setting.family()){
                setting.setFamily(family);
                changed = true;
            }
            if(ui->fontSize->value()!=setting.fontSize()){
                setting.setFontSize(ui->fontSize->value());
                changed = true;
            }
            if(ui->zoom->value()!=setting.fontZoom()){
                setting.setFontZoom(ui->zoom->value());
                changed = true;
            }
            auto scheme = setting.colorSchemeFileName().toString();



            auto item = d->themeData.at(ui->scheme->currentIndex());
            if(scheme!=item.second){
                setting.setColorSchemeFileName(Utils::FilePath::fromString(item.second));
                changed = true;
                *state |= OptionWidget::Restart;//need restart application
            }

            if(changed){
                instance->setFontSettings(setting);
                instance->fontSettingsChanged(setting);
            }

        }
    }
}

void ColorTab::initValue(const QJsonObject& value){

}

QJsonObject ColorTab::toJson(){

    return {};
}

void ColorTab::notifyChanged(const QString& name,const QVariant& value){
    auto instance = TextEditor::TextEditorSettings::instance();
    auto setting = instance->fontSettings();
    auto scheme = setting.colorSchemeFileName().toString();
    qDebug()<<"value"<<value;
    if(name==themeNameKey){
        if(value==-1){
            //resetore light
            d->themeData = TextEditor::FontSettings::schemeMap(0);
            this->resetThemeList(scheme);
        }else if(value==-2){
            //restore dark
            d->themeData = TextEditor::FontSettings::schemeMap(1);
            this->resetThemeList(scheme);
        }else if(value==0){
            //light
            d->themeData = TextEditor::FontSettings::schemeMap(0);
            this->resetThemeList();
        }else if(value==1){
            //dark
            d->themeData = TextEditor::FontSettings::schemeMap(1);
            this->resetThemeList();
        }
    }
}

void ColorTab::initView(){
    auto instance = TextEditor::TextEditorSettings::instance();
    if(instance!=nullptr){
        {
            auto setting = instance->fontSettings();
            ui->font->setCurrentFont(setting.font());
            ui->fontSize->setValue(setting.fontSize());
            ui->zoom->setValue(setting.fontZoom());

            auto environment_settings = OptionsSettings::getInstance()->environmentSettings();

            d->themeData = TextEditor::FontSettings::schemeMap(environment_settings.m_theme==QLatin1String("dark")?1:0);
            int i = 0;
            for(auto one:d->themeData){
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

void ColorTab::resetThemeList(const QString& value){
    auto instance = TextEditor::TextEditorSettings::instance();
    auto setting = instance->fontSettings();
    ui->scheme->clear();
    int i = 0;
    for(auto one:d->themeData){
        ui->scheme->addItem(one.first,one.second);
        if((value.isEmpty()==false && value==one.second)){
            ui->scheme->setCurrentIndex(i);
        }
        i++;
    }
    if(value.isEmpty())
        ui->scheme->setCurrentIndex(0);
}

void ColorTab::onFontChanged(const QFont& font){
    ui->editor->setFont(font);
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
    QString name  = d->themeData.first().first;
    if(index>=0 && index<d->themeData.size()){
        name = d->themeData.at(index).first;
    }
    ui->editor->setColorScheme(name);
}

}
