#include "environment_option_widget.h"
#include "ui_environment_option_widget.h"
#include <QIcon>

namespace ady{

EnvironmentOptionWidget::EnvironmentOptionWidget(QWidget *parent)
    : OptionWidget(parent)
    , ui(new Ui::EnvironmentOptionWidget)
{
    ui->setupUi(this);

    this->setWindowTitle(tr("Environment"));
    this->setWindowIcon(QIcon(":/Resource/icons/Computer_16x.svg"));
}

EnvironmentOptionWidget::~EnvironmentOptionWidget()
{
    delete ui;
}

QString EnvironmentOptionWidget::name(){
    return QLatin1String("core::environment");
}

void EnvironmentOptionWidget::initValue(const QJsonObject& value){


}


QJsonObject EnvironmentOptionWidget::toJson() {

    return {};
}

}
