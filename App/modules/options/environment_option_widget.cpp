#include "environment_option_widget.h"
#include "ui_environment_option_widget.h"
#include <QIcon>
#include <QListView>
#include <QStyledItemDelegate>
#include "components/list_item_delegate.h"
#include "core/options_setting.h"

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
    return QLatin1String("core::environment");
}

void EnvironmentOptionWidget::initValue(const QJsonObject& value){


}


QJsonObject EnvironmentOptionWidget::toJson() {

    return {};
}



void EnvironmentOptionWidget::initView(){
    ui->theme->addItem(tr("Default"));
    ui->language->addItem(tr("English"));
    ui->language->addItem(tr("Chinese"));

    auto instance = OptionsSetting::getInstance();
    ui->theme->setCurrentIndex(instance->toInt(OptionsSetting::Theme));
    ui->language->setCurrentIndex(instance->toInt(OptionsSetting::Language));
    ui->auto_save->setChecked(instance->toBool(OptionsSetting::AutoSave));
    ui->auto_save_interval->setValue(instance->toInt(OptionsSetting::AutoSaveInterval));
    ui->texteditor_file_limit->setValue(instance->toInt(OptionsSetting::TexteditorFileLimit));
}

}
