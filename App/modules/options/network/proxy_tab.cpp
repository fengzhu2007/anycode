#include "proxy_tab.h"
#include "ui_proxy_tab.h"
#include "../options_settings.h"
#include "../network_settings.h"
#include "../option_widget.h"

namespace ady{

ProxyTab::ProxyTab(QWidget* parent):OptionTab(parent),ui(new Ui::ProxyTab) {
    ui->setupUi(this);
    this->setWindowTitle(tr("Proxy"));
    this->initView();
}

ProxyTab::~ProxyTab(){
    delete ui;
}

QString ProxyTab::name(){
    return QLatin1String("proxy");
}

void ProxyTab::apply(int *state){
    auto instance = OptionsSettings::getInstance();
    auto setting = instance->networkSettings();
    bool changed = false;

    if(setting.m_host != ui->host->text()){
        setting.m_host = ui->host->text();
        changed = true;
    }
    if(setting.m_port != ui->port->value()){
        setting.m_port = ui->port->value();
        changed = true;
    }
    if(setting.m_username != ui->username->text()){
        setting.m_username = ui->username->text();
        changed = true;
    }
    if(setting.m_password != ui->password->text()){
        setting.m_password = ui->password->text();
        changed = true;
    }
    if(setting.m_gatewayEnabled != ui->gateway->isChecked()){
        setting.m_gatewayEnabled = ui->gateway->isChecked();
        changed = true;
    }

    if(changed){
        instance->setNetworkSettings(setting);
        *state |= OptionWidget::Restart;
    }
}

void ProxyTab::initValue(const QJsonObject& value){

}

QJsonObject ProxyTab::toJson(){
    return OptionsSettings::getInstance()->networkSettings().toJson();
}

void ProxyTab::initView(){
    auto instance = OptionsSettings::getInstance();
    auto setting = instance->networkSettings();

    ui->host->setText(setting.m_host);
    ui->port->setValue(setting.m_port);
    ui->username->setText(setting.m_username);
    ui->password->setText(setting.m_password);
    ui->gateway->setChecked(setting.m_gatewayEnabled);
}

}
