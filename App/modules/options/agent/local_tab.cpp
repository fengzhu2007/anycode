#include "local_tab.h"
#include "ui_local_tab.h"
#include "../options_settings.h"
#include "../agent_settings.h"
#include "../option_widget.h"
#include "components/select_model.h"
#include "components/list_item_delegate.h"
#include <QAbstractItemView>

namespace ady{

LocalTab::LocalTab(QWidget* parent):OptionTab(parent),ui(new Ui::LocalTab) {
    ui->setupUi(this);
    this->setWindowTitle(tr("Local"));

    this->initView();
}

LocalTab::~LocalTab(){
    delete ui;
}

QString LocalTab::name(){
    return QLatin1String("local");
}

void LocalTab::apply(int *state){
    auto instance = OptionsSettings::getInstance();
    auto setting = instance->agentSettings();
    bool changed = false;

    if(setting.m_port != ui->port->value()){
        setting.m_port = ui->port->value();
        changed = true;
    }
    if(setting.m_runMode != ui->runMode->currentIndex()){
        setting.m_runMode = ui->runMode->currentIndex();
        changed = true;
    }
    if(setting.m_proxyEnabled != ui->enableProxy->isChecked()){
        setting.m_proxyEnabled = ui->enableProxy->isChecked();
        changed = true;
    }

    if(changed){
        instance->setAgentSettings(setting);
        *state |= OptionWidget::Restart;
    }
}

void LocalTab::initValue(const QJsonObject& value){

}

QJsonObject LocalTab::toJson(){
    return OptionsSettings::getInstance()->agentSettings().toJson();
}

void LocalTab::initView(){
    auto instance = OptionsSettings::getInstance();
    auto setting = instance->agentSettings();

    ui->port->setValue(setting.m_port);

    // Populate run mode combo
    {
        auto model = new SelectModel<int>(ui->runMode);
        QList<QPair<int,QString>> list;
        list.append({AgentSettings::Terminal, tr("Run in built-in terminal")});
        list.append({AgentSettings::QProcessMode, tr("Run in background process")});
        model->setDataSource(list);
        ui->runMode->setModel(model);
        ui->runMode->view()->setItemDelegate(new ListItemDelegate(22, ui->runMode));
    }
    ui->runMode->setCurrentIndex(setting.m_runMode);
    ui->enableProxy->setChecked(setting.m_proxyEnabled);
}

}
