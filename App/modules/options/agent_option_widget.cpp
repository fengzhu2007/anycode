#include "agent_option_widget.h"
#include "ui_agent_option_widget.h"
#include "agent/local_tab.h"
#include "agent_settings.h"
#include "options_settings.h"
#include <QIcon>

namespace ady{
class AgentOptionWidgetPrivate{
public:
    QList<OptionTab*> list;
};

AgentOptionWidget::AgentOptionWidget(QWidget *parent)
    : OptionWidget(parent)
    , ui(new Ui::AgentOptionWidget)
{
    d = new AgentOptionWidgetPrivate;
    ui->setupUi(this);

    this->setWindowTitle(tr("Agent"));
    this->setWindowIcon(QIcon(":/Resource/icons/Extension_16x.svg"));
    this->initView();
}

AgentOptionWidget::~AgentOptionWidget()
{
    delete d;
    delete ui;
}

QString AgentOptionWidget::name(){
    return AgentSettings::name();
}

void AgentOptionWidget::apply(int *state){
    for(auto one:d->list){
        one->apply(state);
    }
}

void AgentOptionWidget::initValue(const QJsonObject& value){

}

QJsonObject AgentOptionWidget::toJson() {
    return OptionsSettings::getInstance()->agentSettings().toJson();
}

void AgentOptionWidget::initView(){
    {
        auto tab = new LocalTab(ui->tabWidget);
        ui->tabWidget->addTab(tab,tab->windowTitle());
        d->list<<tab;
    }
}

}
