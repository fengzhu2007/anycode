#include "addon_item_widget.h"
#include "ui_addon_item_widget.h"
#include "addon_manager_model.h"

namespace ady{
AddonItemWidget::AddonItemWidget(QWidget *parent)
    : ListViewItem(parent)
    , ui(new Ui::AddonItemWidget)
{
    ui->setupUi(this);

    connect(ui->disable,&QPushButton::clicked,this,&AddonItemWidget::onDisable);
    connect(ui->install,&QPushButton::clicked,this,&AddonItemWidget::onInstall);
    connect(ui->uninstall,&QPushButton::clicked,this,&AddonItemWidget::onUnInstall);
    ui->uninstall->hide();

}

AddonItemWidget::~AddonItemWidget()
{
    delete ui;
}

void AddonItemWidget::setTitle(const QString& title){
    ui->title->setText(title);
}
void AddonItemWidget::setIcon(const QIcon& icon){
    if(icon.isNull()){
        ui->icon->setPixmap(QIcon(":/Resource/icons/Extension_16x.svg").pixmap({45,45}));
    }else{
        ui->icon->setPixmap(icon.pixmap(45,45));
    }
}
void AddonItemWidget::setDescription(const QString& description){
    ui->description->setText(description);
}

void AddonItemWidget::setInstalled(bool installed){
    if(installed){
        ui->install->hide();
        ui->disable->hide();
        ui->uninstall->hide();
    }else{
        ui->install->hide();
        ui->disable->hide();
        ui->uninstall->hide();
    }
}

void AddonItemWidget::setAddonItem(const AddonItem& item){
    this->setTitle(item.title);
    this->setDescription(item.description);
    this->setIcon(item.icon);
    this->setInstalled(item.installed);
}

void AddonItemWidget::onInstall(){

}

void AddonItemWidget::onUnInstall(){

}

void AddonItemWidget::onDisable(){

}


}
