#include "table_index_widget.h"
#include "ui_table_index_widget.h"


namespace ady{
TableIndexWidget::TableIndexWidget(long long id,const QString& table,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TableIndexWidget)
{
    ui->setupUi(this);
}

TableIndexWidget::~TableIndexWidget()
{
    delete ui;
}

}
