#include "options_dialog.h"
#include "ui_options_dialog.h"
#include "options_model.h"
#include "app_oss.h"
#include "environment_option_widget.h"
#include "texteditor_option_widget.h"
#include "storage/common_storage.h"
#include "components/message_dialog.h"
#include "core/event_bus/publisher.h"
#include "core/event_bus/type.h"
#include <QJsonDocument>
#include <QKeyEvent>
#include <QDebug>

static int Version = 1;
static QString Name = "AnyCode Options";


namespace ady{
OptionsDialog* OptionsDialog::instance=nullptr;
class OptionsDialogPrivate{
public:
    OptionsModel* model;
    QList<OptionWidget*> list;
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
    connect(ui->filter,&QLineEdit::editingFinished,this,&OptionsDialog::onFilter);

    this->resetupUi();

    this->initView();
}

void OptionsDialog::initView(){
    {
        auto widget = new EnvironmentOptionWidget(ui->stacked);
        d->model->appendItem(widget);
        ui->stacked->addWidget(widget);
        d->list <<widget;
    }
    {
        auto widget = new TextEditorOptionWidget(ui->stacked);
        d->model->appendItem(widget);
        ui->stacked->addWidget(widget);
        d->list <<widget;
    }


    if(d->model->rowCount()>0){
        this->setCurrentIndex(0);
        auto index = d->model->index(0,0);
        ui->listView->selectionModel()->select(index,QItemSelectionModel::Select);
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

void OptionsDialog::notifyChanged(const QString& name,const QVariant& value){
    for(auto one:d->list){
        one->notifyChanged(name,value);
    }
}

void OptionsDialog::keyPressEvent(QKeyEvent *e){
    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        e->ignore();
    } else {
        wDialog::keyPressEvent(e);
    }
}

void OptionsDialog::onActivate(const QModelIndex& index){
    this->setCurrentIndex(index.row());
}



void OptionsDialog::onSave(){
    QJsonObject options = {};
    int state = 0;
    for(auto one:d->list){
        one->apply(&state);
        options.insert(one->name(),one->toJson());
    }
    QJsonObject data = {
            {"version",Version},
            {"name",Name},
            {"options",options},
    };
    QJsonDocument doc;
    doc.setObject(data);
    CommonStorage().replace(CommonStorage::OPTIONS,doc.toJson());

    if((state&OptionWidget::Restart)==OptionWidget::Restart){
        //restart application
        if(MessageDialog::confirm(this,tr("Restart required"),tr("The settings are modified and need to be restarted to take effect; Restart now?"))==QMessageBox::Yes){
            Publisher::getInstance()->post(Type::M_RESTART);
            return ;
        }
    }
    this->close();
}

void OptionsDialog::onFilter(){
    const QString text = ui->filter->text();
    d->model->setDataSource(d->list);
    d->model->filter(text);
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

