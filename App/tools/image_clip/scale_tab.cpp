#include "scale_tab.h"
#include "ui_scale_tab.h"
namespace ady{
ScaleTab::ScaleTab(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ScaleTab)
{
    ui->setupUi(this);
}

ScaleTab::~ScaleTab()
{
    delete ui;
}

int ScaleTab::optionWidth(){
    return ui->width->value();
}
int ScaleTab::optionHeight(){
    return ui->height->value();
}
}
