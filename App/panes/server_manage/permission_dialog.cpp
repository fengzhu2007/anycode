#include "permission_dialog.h"
#include "ui_permission_dialog.h"

namespace ady{

PermissionDialog::PermissionDialog(int mode,QWidget* parent):
    wDialog(parent),ui(new Ui::PermissionDialog)
{
    ui->setupUi(this);


    connect(ui->okButton,&QPushButton::clicked,this,&PermissionDialog::onOK);
    connect(ui->cancelButton,&QPushButton::clicked,this,&PermissionDialog::close);
    //qDebug()<<"mode:"<<mode;
    int a = mode/100;
    mode -= (a*100);
    int b = mode/10;
    mode -= (b*10);
    int c = mode;
    //qDebug()<<"a:"<<a<<";b:"<<b<<";c:"<<c;
    int r=4,w=2,x=1;

    if((a&r)==r){
        ui->permission1Checkbox->setChecked(true);
    }
    if((a&w)==w){
        ui->permission2Checkbox->setChecked(true);
    }
    if((a&x)==x){
        ui->permission3Checkbox->setChecked(true);
    }



    if((b&r)==r){
        ui->permission4Checkbox->setChecked(true);
    }
    if((b&w)==w){
        ui->permission5Checkbox->setChecked(true);
    }
    if((b&x)==x){
        ui->permission6Checkbox->setChecked(true);
    }

    if((c&r)==r){
        ui->permission7Checkbox->setChecked(true);
    }
    if((c&w)==w){
        ui->permission8Checkbox->setChecked(true);
    }
    if((c&x)==x){
        ui->permission9Checkbox->setChecked(true);
    }
}


PermissionDialog::~PermissionDialog(){
    delete d;
    delete ui;
}

int PermissionDialog::mode()
{
    int a=0,b=0,c=0;
    //a |= 2;
    //a |= 4;
    if(ui->permission1Checkbox->isChecked()){
        a |= 4;
    }
    if(ui->permission2Checkbox->isChecked()){
        a |= 2;
    }
    if(ui->permission3Checkbox->isChecked()){
        a |= 1;
    }

    if(ui->permission4Checkbox->isChecked()){
        b |= 4;
    }
    if(ui->permission5Checkbox->isChecked()){
        b |= 2;
    }
    if(ui->permission6Checkbox->isChecked()){
        b |= 1;
    }
    if(ui->permission7Checkbox->isChecked()){
        c |= 4;
    }
    if(ui->permission8Checkbox->isChecked()){
        c |= 2;
    }
    if(ui->permission9Checkbox->isChecked()){
        c |= 1;
    }
    return a*100 + b*10 + c;
}

bool PermissionDialog::applyChildren()
{
    return ui->applyChildrenCheckbox->isChecked();
}

void PermissionDialog::onOK()
{
    this->accept();
}

}
