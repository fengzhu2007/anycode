#include "php_tab.h"
#include "ui_php_tab.h"
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
#include "../language_settings.h"


namespace ady{

PHPTab::PHPTab(QWidget *parent)
    : OptionTab(parent)
    , ui(new Ui::PHPTab)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("PHP"));
    this->initView();

    connect(ui->phpCmdChecking,&QCheckBox::stateChanged,this,&PHPTab::onStateChanged);
}

PHPTab::~PHPTab()
{
    delete ui;
}

QString PHPTab::name(){
    return QLatin1String("php");
}

void PHPTab::apply(int *state){
    auto instance = OptionsSettings::getInstance();
    auto setting = instance->languageSettings();
    bool changed = false;
    if(setting.m_phpCmdChecking!=ui->phpCmdChecking->isChecked()){
        setting.m_phpCmdChecking = ui->phpCmdChecking->isChecked();
        changed = true;
    }

    if(setting.m_phpCmdPath!=ui->phpCmdPath->text()){
        setting.m_phpCmdPath = ui->phpCmdPath->text();
        changed = true;
    }
    if(changed){
        instance->setLanguageSettings(setting);
    }
}

void PHPTab::initValue(const QJsonObject& value){

}

QJsonObject PHPTab::toJson(){

    return {};
}

void PHPTab::notifyChanged(const QString& name,const QVariant& value){

}

void PHPTab::initView(){
    ui->phpCmdPath->setEnable(true);
#if defined(Q_OS_WIN)
    ui->phpCmdPath->setFilter("Executable Files (*.exe);;All Files (*)");
#else
    ui->phpCmdPath->setFilter("All Files (*)");
#endif
    auto instance = OptionsSettings::getInstance();
    auto setting = instance->languageSettings();
    ui->phpCmdChecking->setChecked(setting.m_phpCmdChecking);
    ui->phpCmdPath->setText(setting.m_phpCmdPath);
    this->onStateChanged(ui->phpCmdChecking->checkState());
}

void PHPTab::onStateChanged(int state){
    ui->phpCmdPath->setEnable(state==Qt::Checked);
}


}
