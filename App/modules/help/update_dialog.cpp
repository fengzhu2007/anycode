#include "update_dialog.h"
#include "modules/help/ui_update_dialog.h"
#include "ui_update_dialog.h"


namespace ady{
UpdateDialog* UpdateDialog::instance = nullptr;
UpdateDialog::UpdateDialog(QWidget *parent)
    : wDialog(parent)
    , ui(new Ui::UpdateDialog)
{
    ui->setupUi(this);
    this->resetupUi();
}


void UpdateDialog::closeEvent(QCloseEvent* e){
    wDialog::closeEvent(e);
}

UpdateDialog::~UpdateDialog()
{
    delete ui;
    instance = nullptr;
}

UpdateDialog* UpdateDialog::open(QWidget* parent){
    if(instance==nullptr){
        instance = new UpdateDialog(parent);
    }
    instance->show();
    return instance;
}

}
