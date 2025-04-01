#include "table_list_widget.h"
#include "ui_table_list_widget.h"



namespace ady{

TableListWidget::TableListWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TableListWidget)
{
    ui->setupUi(this);
}

TableListWidget::~TableListWidget()
{
    delete ui;
}


}
