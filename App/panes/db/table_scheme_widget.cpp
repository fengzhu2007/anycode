#include "table_scheme_widget.h"
#include "ui_table_scheme_widget.h"



namespace ady{
TableSchemeWidget::TableSchemeWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TableSchemeWidget)
{
    ui->setupUi(this);
}

TableSchemeWidget::~TableSchemeWidget()
{
    delete ui;
}
}
