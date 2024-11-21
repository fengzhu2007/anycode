#include "general_tab.h"
#include "ui_general_tab.h"
#include "components/list_item_delegate.h"
#include <texteditorsettings.h>
#include <behaviorsettings.h>
#include <fontsettings.h>
#include <typingsettings.h>
#include <tabsettings.h>
#include <displaysettings.h>
#include <extraencodingsettings.h>


namespace ady{
GeneralTab::GeneralTab(QWidget* parent):OptionTab(parent),ui(new Ui::GeneralTab) {

    ui->setupUi(this);
    this->setWindowTitle(tr("General"));
    this->initView();
}


GeneralTab::~GeneralTab(){
    delete ui;
}

QString GeneralTab::name(){
    return QLatin1String("general");
}

void GeneralTab::initValue(const QJsonObject& value){

}

QJsonObject GeneralTab::toJson(){

    return {};
}

void GeneralTab::initView(){
    auto instance = TextEditor::TextEditorSettings::instance();
    if(instance!=nullptr){
        {
            /*auto setting = instance->fontSettings();
            ui->font->setCurrentFont(setting.font());
            ui->fontSize->setValue(setting.fontSize());
            ui->zoom->setValue(setting.fontZoom());*/
        }

        {
            ui->tabPolicy->addItem(tr("Spaces Only"));
            ui->tabPolicy->addItem(tr("Tabs Only"));
            ui->tabPolicy->addItem(tr("Mixed"));
            ui->tabPolicy->setItemDelegate(new ListItemDelegate(22,ui->tabPolicy));
            auto setting = TextEditor::TabSettings{};
            ui->indentSize->setValue(setting.m_indentSize);
            ui->tabSize->setValue(setting.m_tabSize);
            ui->tabPolicy->setCurrentIndex(setting.m_tabPolicy);
        }

        {
            auto setting = instance->displaySettings();
            ui->auto_wrapping->setChecked(setting.m_textWrapping);
            ui->display_whitespace->setChecked(setting.m_visualizeWhitespace);
            ui->display_line_number->setChecked(setting.m_displayLineNumbers);
            ui->hightlight_current_line->setChecked(setting.m_highlightCurrentLine);

        }

        {
            auto setting = instance->extraEncodingSettings();
            ui->charset->addItem(tr("System"),QLatin1String("System"));
            ui->charset->addItem(QLatin1String("UTF-8"),QLatin1String("UTF-8"));
            ui->charset->setItemDelegate(new ListItemDelegate(22,ui->charset));


            ui->utf8bom->addItem(tr("Always Add"));
            ui->utf8bom->addItem(tr("Only Keep"));
            ui->utf8bom->addItem(tr("Always Delete"));
            ui->utf8bom->setItemDelegate(new ListItemDelegate(22,ui->utf8bom));

            ui->utf8bom->setCurrentIndex(setting.m_utf8BomSetting);

            //lineTerminationModeNames
            auto list = TextEditor::ExtraEncodingSettings::lineTerminationModeNames();

            ui->lineEndings->setItemDelegate(new ListItemDelegate(22,ui->lineEndings));
            for(auto one:list){
                ui->lineEndings->addItem(one);
            }
#ifdef Q_OS_WIN
            ui->lineEndings->setCurrentIndex(1);
#elif
            ui->lineEndings->setCurrentIndex(0);
#endif

        }
    }
}


}
