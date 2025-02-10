#include "ssl_query_dialog.h"
#include "ui_ssl_query_dialog.h"
#include "core/schedule/schedule.h"
#include "ssl_query_task.h"

#include <QLineEdit>


namespace ady{

SSLQueryDialog* SSLQueryDialog::instance = nullptr;
class SSLQueryDialogPrivate{
public:
    QLineEdit* editor;

};

SSLQueryDialog::SSLQueryDialog(QWidget* parent):wDialog(parent),ui(new Ui::SSLQueryDialog) {
    d = new SSLQueryDialogPrivate;
    ui->setupUi(this);
    this->resetupUi();


    this->onQuery();
}

SSLQueryDialog::~SSLQueryDialog(){
    delete ui;
    delete d;
    instance = nullptr;
}

SSLQueryDialog* SSLQueryDialog::getInstance(){
    return instance;
}

SSLQueryDialog* SSLQueryDialog::open(QWidget* parent){
    if(instance==nullptr){
        instance = new SSLQueryDialog(parent);
    }
    instance->show();
    return instance;
}

void SSLQueryDialog::onQuery(){
    auto instance = Schedule::getInstance();
    //https://w.guoqinwang.xiyogo.com/
    instance->addTask(new SSLQueryTask({"w.guoqinwang.xiyogo.com"}));
}

}
