#include "replace_all_progress_dialog.h"
#include "ui_replace_all_progress_dialog.h"


namespace ady{

ReplaceAllProgressDialog* ReplaceAllProgressDialog::instance = nullptr;

ReplaceAllProgressDialog::ReplaceAllProgressDialog(QWidget *parent) :
    wDialog(parent),
    ui(new Ui::ReplaceAllProgressDialog)
{
    ui->setupUi(this);

    this->resetupUi();

    this->hideNClient();
    ui->widget->start();
    connect(ui->cancel,&QPushButton::clicked,this,&QDialog::reject);
}

ReplaceAllProgressDialog::~ReplaceAllProgressDialog()
{
    delete ui;
    instance = nullptr;
}

ReplaceAllProgressDialog* ReplaceAllProgressDialog::open(QWidget* parent){
    if(instance==nullptr){
        instance = new ReplaceAllProgressDialog(parent);
    }
    instance->setModal(true);
    instance->show();
    return instance;
}

}
