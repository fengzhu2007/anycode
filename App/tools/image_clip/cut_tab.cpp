#include "cut_tab.h"
#include "ui_cut_tab.h"
namespace ady{
CutTab::CutTab(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CutTab)
{
    ui->setupUi(this);
}

CutTab::~CutTab()
{
    delete ui;
}


int CutTab::optionLeft(){
    return ui->left->value();
}

int CutTab::optionTop(){
    return ui->top->value();
}

int CutTab::optionRight(){
    return ui->right->value();
}

int CutTab::optionBottom(){
    return ui->bottom->value();
}

int CutTab::optionWidth(){
    return ui->width->value();
}

int CutTab::optionHeight(){
    return ui->height->value();
}


}
