#include "ChmodDialog.h"
#include "ui_ChmodDialog.h"
#include <QDebug>
namespace ady {

ChmodDialog::ChmodDialog(int mode,QWidget* parent)
    :QDialog(parent),
      ui(new Ui::ChmodDialogUI)
{
    ui->setupUi(this);
    connect(ui->okButton,&QPushButton::clicked,this,&ChmodDialog::onOK);
    connect(ui->cancelButton,&QPushButton::clicked,this,&ChmodDialog::close);
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

ChmodDialog::~ChmodDialog()
{
    delete ui;
}

int ChmodDialog::mode()
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

bool ChmodDialog::applyChildren()
{
    return ui->applyChildrenCheckbox->isChecked();
}

void ChmodDialog::setFileInfo(QString file,int count)
{
    if(count>1){
        ui->fileLabel->setText(tr("File/Folder:%1;%2 files, etc").arg(file).arg(count));
    }else{
        ui->fileLabel->setText(tr("File/Folder:%1").arg(file));
    }
}

int ChmodDialog::toFormat(QString permission)
{
    int a=0,b=0,c=0;
    int r=4,w=2,x=1;
    if(permission.isEmpty()==false){
        if(permission[0]=='0'){
            int index = 1;
            if(permission.size()==5){
                index++;
            }
            a = permission.mid(index++,1).toInt();
            b = permission.mid(index++,1).toInt();
            c = permission.mid(index++,1).toInt();
            //qDebug()<<"format a:"<<a<<";b:"<<b<<";c:"<<c;
        }else if(permission[0]=='d'||permission[0]=='-'||permission[0]=='l'){
            if(permission[1]=='r'){
                a |= r;
            }
            if(permission[2]=='w'){
                a |= w;
            }
            if(permission[3]=='x'){
                a |= x;
            }
            if(permission[4]=='r'){
                b |= r;
            }
            if(permission[5]=='w'){
                b |= w;
            }
            if(permission[6]=='x'){
                b |= x;
            }
            if(permission[7]=='r'){
                c |= r;
            }
            if(permission[8]=='w'){
                c |= w;
            }
            if(permission[9]=='x'){
                c |= x;
            }
        }
    }
    int mode = a*100 + b*10 + c;
    return mode;
}

void ChmodDialog::onOK()
{
    this->accept();
}


}
