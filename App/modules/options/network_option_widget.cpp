#include "network_option_widget.h"
#include "ui_network_option_widget.h"
#include "network/proxy_tab.h"
#include "network_settings.h"
#include "options_settings.h"
#include <QIcon>

namespace ady{
class NetworkOptionWidgetPrivate{
public:
    QList<OptionTab*> list;
};

NetworkOptionWidget::NetworkOptionWidget(QWidget *parent)
    : OptionWidget(parent)
    , ui(new Ui::NetworkOptionWidget)
{
    d = new NetworkOptionWidgetPrivate;
    ui->setupUi(this);

    this->setWindowTitle(tr("Network"));
    this->setWindowIcon(QIcon(":/Resource/icons/ConnectToRemoteServer_16x.svg"));
    this->initView();
}

NetworkOptionWidget::~NetworkOptionWidget()
{
    delete d;
    delete ui;
}

QString NetworkOptionWidget::name(){
    return NetworkSettings::name();
}

void NetworkOptionWidget::apply(int *state){
    for(auto one:d->list){
        one->apply(state);
    }
}

void NetworkOptionWidget::initValue(const QJsonObject& value){

}

QJsonObject NetworkOptionWidget::toJson() {
    return OptionsSettings::getInstance()->networkSettings().toJson();
}

void NetworkOptionWidget::initView(){
    {
        auto tab = new ProxyTab(ui->tabWidget);
        ui->tabWidget->addTab(tab,tab->windowTitle());
        d->list<<tab;
    }
}

}
