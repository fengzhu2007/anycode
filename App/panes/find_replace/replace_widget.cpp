#include "replace_widget.h"
#include "ui_replace_widget.h"
namespace ady{
ReplaceWidget::ReplaceWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ReplaceWidget)
{
    ui->setupUi(this);
}

ReplaceWidget::~ReplaceWidget()
{
    delete ui;
}
}
