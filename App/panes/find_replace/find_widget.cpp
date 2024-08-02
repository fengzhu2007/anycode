#include "find_widget.h"
#include "ui_find_widget.h"
#include "find_replace_dialog.h"
#include <QDebug>
namespace ady {

FindWidget::FindWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FindWidget)
{
    ui->setupUi(this);

    connect(ui->findPrevious,&QPushButton::clicked,this,&FindWidget::onClicked);
    connect(ui->findNext,&QPushButton::clicked,this,&FindWidget::onClicked);
    connect(ui->findAll,&QPushButton::clicked,this,&FindWidget::onClicked);
    connect(ui->selectFolder,&QPushButton::clicked,this,&FindWidget::onSelectFolder);
}

FindWidget::~FindWidget()
{
    delete ui;
}
/*
    FindBackward = 0x01,
    FindCaseSensitively = 0x02,
    FindWholeWords = 0x04,
    FindRegularExpression = 0x08,
    FindPreserveCase = 0x10
*/
void FindWidget::onClicked(){
    auto dialog = static_cast<FindReplaceDialog*>(parentWidget()->parentWidget()->parentWidget());
    auto sender = static_cast<QPushButton*>(this->sender());
    int bit = 0;
    if(ui->caseSensitive->isChecked()){
        bit |= 0x02;
    }
    if(ui->wholeWords->isChecked()){
        bit |= 0x04;
    }
    if(ui->regular->isChecked()){
        bit |= 0x08;
    }
    const QString text = ui->find->currentText();
    if(sender==ui->findPrevious){
        bit |= 0x01;
        dialog->search(text,bit);
    }else if(sender==ui->findNext){
        dialog->search(text,bit);
    }else if(sender==ui->findAll){

    }
}

void FindWidget::onSelectFolder(){

}
}
