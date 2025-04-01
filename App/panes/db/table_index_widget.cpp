#include "table_index_widget.h"
#include "ui_table_index_widget.h"

TableIndexWidget::TableIndexWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TableIndexWidget)
{
    ui->setupUi(this);
}

TableIndexWidget::~TableIndexWidget()
{
    delete ui;
}
