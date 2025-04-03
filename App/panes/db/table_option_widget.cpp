#include "table_option_widget.h"
#include "ui_table_option_widget.h"

namespace ady{
TableOptionWidget::TableOptionWidget(long long id,const QString& table,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TableOptionWidget)
{
    ui->setupUi(this);
}

TableOptionWidget::~TableOptionWidget()
{
    delete ui;
}
}
