#include "sqlite_connect_dialog.h"
#include "ui_sqlite_connect_dialog.h"
#include "core/theme.h"
#include "storage/db_storage.h"
#include "components/message_dialog.h"
#include "core/event_bus/publisher.h"
#include "core/event_bus/type.h"

namespace ady{
class SQLiteConnectDialogPrivate{
public:
    explicit SQLiteConnectDialogPrivate(const DBRecord& record):data(record){

    }

    DBRecord data;
};

SQLiteConnectDialog::SQLiteConnectDialog(const DBRecord& data,QWidget *parent)
    : wDialog(parent)
    , ui(new Ui::SQLiteConnectDialog)
{

    d = new SQLiteConnectDialogPrivate(data);
    auto theme = Theme::getInstance();
    auto backgroundColor = theme->backgroundColor().name(QColor::HexRgb);
    this->setStyleSheet("QPushButton{height:20px}"
                        "#footer{background:"+backgroundColor+"}");

    ui->setupUi(this);
    this->resetupUi();
    setAttribute(Qt::WA_DeleteOnClose);


    //connect
    connect(ui->type,QOverload<QAbstractButton*,bool>::of(&QButtonGroup::buttonToggled),this,&SQLiteConnectDialog::onTypeChanged);
    connect(ui->ok,&QPushButton::clicked,this,&SQLiteConnectDialog::onSave);
    connect(ui->test,&QPushButton::clicked,this,&SQLiteConnectDialog::onTest);
    connect(ui->cancel,&QPushButton::clicked,this,&SQLiteConnectDialog::close);

    this->initView();

}

SQLiteConnectDialog::~SQLiteConnectDialog()
{
    delete ui;
}

void SQLiteConnectDialog::initView(){
    ui->selectFile->setChecked(true);
}


bool SQLiteConnectDialog::validate(){
    auto name = ui->name->text();
    if(name.isEmpty()){
        MessageDialog::error(this,tr("Name required."));
        return false;
    }
    auto file = ui->file->text();
    if(file.isEmpty()){
        MessageDialog::error(this,tr("File required."));
        return false;
    }
    d->data.name = name;
    d->data.host = file;
    d->data.username = ui->username->text();
    d->data.password = ui->password->text();

    return true;
}

void SQLiteConnectDialog::onTypeChanged(QAbstractButton *button, bool checked){
    if(checked){
        if(button==ui->selectFile){
            ui->username->setEnabled(true);
            ui->password->setEnabled(true);
            ui->savePassword->setEnabled(true);
        }else{
            //new file
            ui->username->setEnabled(false);
            ui->password->setEnabled(false);
            ui->savePassword->setEnabled(false);
        }
    }
}


void SQLiteConnectDialog::onSave(){
    if(this->validate()){
        if(d->data.id==0l){
            //insert
            d->data.driver = "sqlite";
            auto ret = DBStorage().insert(d->data);
            if(ret>0l){
                d->data.id = ret;
                //pust
                auto instance = Publisher::getInstance();
                instance->post(Type::M_NEW_CONNECTION,d->data.toJson());
            }
        }else{
            auto ret = DBStorage().update(d->data);
            if(ret){
                auto instance = Publisher::getInstance();
                instance->post(Type::M_UPDATE_CONNECTION,d->data.toJson());
            }
        }
        this->close();
    }
}

void SQLiteConnectDialog::onTest(){
    if(this->validate()){

    }
}

}
