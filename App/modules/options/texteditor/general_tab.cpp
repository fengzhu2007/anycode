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
#include <completionsettings.h>


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

void GeneralTab::apply(int *state){
    auto instance = TextEditor::TextEditorSettings::instance();
    if(instance!=nullptr){
        {
            bool changed = false;
            auto setting = instance->completionSettings();
            if(ui->completionTrigger->currentIndex()!=setting.m_completionTrigger){
                setting.m_completionTrigger = (TextEditor::CompletionTrigger)ui->completionTrigger->currentIndex();
                changed = true;
            }
            if(ui->caseSensitivity->currentIndex()!=setting.m_caseSensitivity){
                setting.m_caseSensitivity = (TextEditor::CaseSensitivity)ui->caseSensitivity->currentIndex();
                changed = true;
            }
            if(ui->timeout->value()!=setting.m_automaticProposalTimeoutInMs){
                setting.m_automaticProposalTimeoutInMs = ui->timeout->value();
                changed = true;
            }
            if(ui->characterThreshold->value()!=setting.m_characterThreshold){
                setting.m_characterThreshold = ui->characterThreshold->value();
                changed = true;
            }
            if(changed){
                instance->setCompletionSettings(setting);
                instance->completionSettingsChanged(setting);
            }
        }

        {
            bool changed = false;
            auto setting = instance->extraEncodingSettings();
            if(ui->charset->currentText()!=setting.m_defaultCharset){
                setting.m_defaultCharset = ui->charset->currentText();
                changed = true;
            }
            if(ui->utf8bom->currentIndex()!=setting.m_utf8BomSetting){
                setting.m_utf8BomSetting = (TextEditor::ExtraEncodingSettings::Utf8BomSetting)ui->charset->currentIndex();
                changed = true;
            }
            if(ui->lineEndings->currentIndex()!=setting.m_lineEnding){
                setting.m_lineEnding = (TextEditor::ExtraEncodingSettings::LineEnding)ui->lineEndings->currentIndex();
                changed = true;
            }
            if(changed){
                instance->setExtraEncodingSettings(setting);
                instance->extraEncodingSettingsChanged(setting);

            }

        }

        {
            bool changed = false;
            auto setting = instance->displaySettings();
            if(ui->auto_wrapping->isChecked()!=setting.m_textWrapping){
                setting.m_textWrapping = ui->auto_wrapping->isChecked();
                changed = true;
            }

            if(ui->display_line_number->isChecked()!=setting.m_displayLineNumbers){
                setting.m_displayLineNumbers = ui->display_line_number->isChecked();
                changed = true;
            }
            if(ui->display_whitespace->isChecked()!=setting.m_visualizeWhitespace){
                setting.m_visualizeWhitespace = ui->display_whitespace->isChecked();
                changed = true;
            }
            if(ui->hightlight_current_line->isChecked()!=setting.m_highlightCurrentLine){
                setting.m_visualizeWhitespace = ui->hightlight_current_line->isChecked();
                changed = true;
            }

            if(changed){
                instance->setDisplaySettings(setting);
                instance->displaySettingsChanged(setting);
            }

        }

        {
            bool changed = false;
            TextEditor::TabSettings setting;
            if(ui->tabPolicy->currentIndex()!=setting.m_tabPolicy){
                setting.m_tabPolicy = (TextEditor::TabSettings::TabPolicy)ui->tabPolicy->currentIndex();
                changed = true;
            }
            if(ui->tabSize->value()!=setting.m_tabSize){
                setting.m_tabSize = ui->tabSize->value();
                changed = true;
            }
            if(ui->indentSize->value()!=setting.m_indentSize){
                setting.m_indentSize = ui->indentSize->value();
                changed = true;
            }
            if(changed)
                TextEditor::TabSettings::initGlobal(setting);
        }

    }
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
            auto setting = instance->completionSettings();
            ui->caseSensitivity->addItem(tr("Case Insensitive"));
            ui->caseSensitivity->addItem(tr("Case Sensitive"));
            ui->caseSensitivity->addItem(tr("First Letter Case Sensitive"));
            ui->caseSensitivity->setItemDelegate(new ListItemDelegate(22,ui->caseSensitivity));
            ui->caseSensitivity->setCurrentIndex(setting.m_caseSensitivity);

            ui->completionTrigger->addItem(tr("Manual Completion"));
            ui->completionTrigger->addItem(tr("Triggered Completion"));
            ui->completionTrigger->addItem(tr("Automatic Completion"));
            ui->completionTrigger->setItemDelegate(new ListItemDelegate(22,ui->completionTrigger));
            ui->completionTrigger->setCurrentIndex(setting.m_completionTrigger);

            ui->timeout->setValue(setting.m_automaticProposalTimeoutInMs);
            ui->characterThreshold->setValue(setting.m_characterThreshold);
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

            {
                int i = 0;
                const QStringList list = {"System","UTF-8","UTF-16","UTF-32"};
                for(auto one:list){
                    ui->charset->addItem(one);
                    if(setting.m_defaultCharset==one){
                        ui->charset->setCurrentIndex(i);
                    }
                    ++i;
                }
            }

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
            ui->lineEndings->setCurrentIndex(setting.m_lineEnding);


        }
    }
}


}
