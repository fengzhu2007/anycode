#include "table_option_widget.h"
#include "ui_table_option_widget.h"

TableOptionWidget::TableOptionWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TableOptionWidget)
{
    ui->setupUi(this);
}

TableOptionWidget::~TableOptionWidget()
{
    delete ui;
}
