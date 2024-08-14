#include "goto_line_dialog.h"
#include "ui_gotolinedialog.h"
#include "core/event_bus/publisher.h"
#include "core/event_bus/type.h"
#include "w_toast.h"
#include <QDebug>
namespace ady{
GotoLineDialog* GotoLineDialog::instance = nullptr;

class GotoLineDialogPrivate{
public:
    int maxLine=-1;
};


GotoLineDialog::GotoLineDialog(QWidget *parent) :
    wDialog(parent),
    ui(new Ui::GotoLineDialog)
{
    d = new GotoLineDialogPrivate;
    ui->setupUi(this);


    connect(ui->lineNo,&QLineEdit::returnPressed,this,&GotoLineDialog::onGoto);
    connect(ui->ok,&QPushButton::clicked,this,&GotoLineDialog::onGoto);
    connect(ui->cancel,&QPushButton::clicked,this,&GotoLineDialog::close);


    this->resetupUi();
}

GotoLineDialog::~GotoLineDialog()
{
    delete d;
    instance = nullptr;
    delete ui;
}

void GotoLineDialog::setLineRange(int max){
    d->maxLine = max;
    ui->label->setText(tr("Line Number(1 - %1):").arg(max));
}

void GotoLineDialog::setCurrentLineNo(int no){
    QString number = QString::fromUtf8("%1").arg(no);
    ui->lineNo->setText(number);
    ui->lineNo->setSelection(0,number.length());
}

void GotoLineDialog::onGoto(){
    const QString no = ui->lineNo->text();
    if(no.isEmpty()==false){
        bool ret;
        int lineNo = no.toInt(&ret);
        //qDebug()<<"ret:"<<ret;
        if(ret && lineNo<=d->maxLine && lineNo>=1){
            Publisher::getInstance()->post(Type::M_GOTO,&lineNo);
            this->close();
        }else{
            //invalid line number
            wToast::showText(tr("Invalid Line Number"));
        }
    }
}

GotoLineDialog* GotoLineDialog::getInstance(){
    return instance;
}

GotoLineDialog* GotoLineDialog::open(QWidget* parent,int current,int max){
    if(instance==nullptr){
        instance = new GotoLineDialog(parent);
    }
    instance->setModal(true);
    instance->setLineRange(max);
    instance->setCurrentLineNo(current);
    instance->show();
    return instance;
}

}
