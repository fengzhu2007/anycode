#include "environment_option_widget.h"
#include "ui_environment_option_widget.h"
#include <QIcon>
#include <QListView>
#include <QStyledItemDelegate>
#include "components/list_item_delegate.h"
#include "options_settings.h"
#include "environment_settings.h"

namespace ady{



EnvironmentOptionWidget::EnvironmentOptionWidget(QWidget *parent)
    : OptionWidget(parent)
    , ui(new Ui::EnvironmentOptionWidget)
{
    ui->setupUi(this);

    this->setWindowTitle(tr("Environment"));
    this->setWindowIcon(QIcon(":/Resource/icons/Computer_16x.svg"));


    ui->theme->view()->setItemDelegate(new ListItemDelegate(22,ui->theme));
    ui->language->view()->setItemDelegate(new ListItemDelegate(22,ui->language));

    this->initView();
}

EnvironmentOptionWidget::~EnvironmentOptionWidget()
{
    delete ui;
}

QString EnvironmentOptionWidget::name(){
    return EnvironmentSettings::name();
}

void EnvironmentOptionWidget::apply(){
    auto instance = OptionsSettings::getInstance();
    auto setting = instance->environmentSettings();
    bool changed = false;
    {
        int index = ui->theme->currentIndex();
        auto themes = EnvironmentSettings::themes();
        if(index>=0 && index<themes.size()){
            auto one = themes.at(index);
            if(setting.m_theme!=one.first){
                setting.m_theme = one.first;
                changed = true;
            }
        }
    }
    {
        int index = ui->language->currentIndex();
        auto languages = EnvironmentSettings::languages();
        if(index>=0 && index<languages.size()){
            auto one = languages.at(index);
            if(setting.m_language!=one.first){
                setting.m_language = one.first;
                changed = true;
            }
        }
    }
    if(setting.m_autoSave!=ui->auto_save->isChecked()){
        setting.m_autoSave = ui->auto_save->isChecked();//apply
        changed = true;
    }
    if(setting.m_autoSaveInterval!=ui->auto_save_interval->value()){
        setting.m_autoSaveInterval = ui->auto_save_interval->value();
        changed = true;
    }
    if(setting.m_texteditorFileLimit!=ui->texteditor_file_limit->value()){
        setting.m_texteditorFileLimit = ui->texteditor_file_limit->value();
        changed = true;
    }
    if(changed){
        instance->setEnvironmentSettings(setting);
    }
}

void EnvironmentOptionWidget::initValue(const QJsonObject& value){


}


QJsonObject EnvironmentOptionWidget::toJson() {
    return OptionsSettings::getInstance()->environmentSettings().toJson();
}



void EnvironmentOptionWidget::initView(){
    auto instance = OptionsSettings::getInstance();
    auto setting = instance->environmentSettings();

    auto themes = EnvironmentSettings::themes();
    int i = 0;
    for(auto one:themes){
        ui->theme->addItem(one.second);
        if(setting.m_theme==one.first){
            ui->theme->setCurrentIndex(i);
        }
        i++;
    }

    i = 0;
    auto languages = EnvironmentSettings::languages();
    for(auto one:languages){
        ui->language->addItem(one.second);
        if(setting.m_language==one.first){
            ui->language->setCurrentIndex(i);
        }
        i++;
    }

    ui->auto_save->setChecked(setting.m_autoSave);
    ui->auto_save_interval->setValue(setting.m_autoSaveInterval);
    ui->texteditor_file_limit->setValue(setting.m_texteditorFileLimit);

}

}
