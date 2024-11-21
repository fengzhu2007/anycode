#include "options_dialog.h"
#include "ui_options_dialog.h"
#include "options_model.h"
#include "addon_loader.h"
#include "app_oss.h"

#include "environment_option_widget.h"
#include "texteditor_option_widget.h"


namespace ady{
OptionsDialog* OptionsDialog::instance=nullptr;
class OptionsDialogPrivate{
public:
    OptionsModel* model;
};

OptionsDialog::OptionsDialog(QWidget* parent):wDialog(parent),ui(new Ui::OptionsDialog) {


    this->setStyleSheet(AppOSS::options());
    ui->setupUi(this);
    d = new OptionsDialogPrivate;
    d->model = new OptionsModel(ui->listView);
    ui->listView->setModel(d->model);
    ui->listView->setFocusPolicy(Qt::NoFocus);
    connect(ui->listView,&QListView::clicked,this,&OptionsDialog::onActivate);

    connect(ui->ok,&QPushButton::clicked,this,&OptionsDialog::onSave);
    connect(ui->cancel,&QPushButton::clicked,this,&OptionsDialog::close);

    this->resetupUi();

    this->initView();
}

void OptionsDialog::initView(){
    {
        auto widget = new EnvironmentOptionWidget(ui->stacked);
        d->model->appendItem(widget);
        ui->stacked->addWidget(widget);
    }
    {
        auto widget = new TextEditorOptionWidget(ui->stacked);
        d->model->appendItem(widget);
        ui->stacked->addWidget(widget);
    }


    if(d->model->rowCount()>0){
        this->setCurrentIndex(0);
    }
}

OptionsDialog::~OptionsDialog(){
    delete d;
    delete ui;
}

void OptionsDialog::setCurrentIndex(int row){
    ui->stacked->setCurrentIndex(row);
    auto widget = d->model->at(row);
    ui->label->setText(widget->windowTitle());
}

void OptionsDialog::onActivate(const QModelIndex& index){
    this->setCurrentIndex(index.row());
}

void OptionsDialog::onSave(){

}

OptionsDialog* OptionsDialog::open(QWidget* parent){
    if(instance==nullptr){
        instance = new OptionsDialog(parent);
    }
    instance->setModal(true);
    instance->show();
    return instance;
}


}

