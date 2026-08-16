#include "resize_tab.h"
#include "ui_resize_tab.h"
namespace ady{
ResizeTab::ResizeTab(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ResizeTab)
{
    ui->setupUi(this);
}

ResizeTab::~ResizeTab()
{
    delete ui;
}


int ResizeTab::optionLeft(){
    return ui->left->value();
}

int ResizeTab::optionTop(){
    return ui->top->value();
}

int ResizeTab::optionRight(){
    return ui->right->value();
}

int ResizeTab::optionBottom(){
    return ui->bottom->value();
}

int ResizeTab::optionWidth(){
    return ui->width->value();
}

int ResizeTab::optionHeight(){
    return ui->height->value();
}

bool ResizeTab::optionRelative(){
    return ui->relative->isChecked();
}

}
