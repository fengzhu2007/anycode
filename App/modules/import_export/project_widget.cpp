#include "project_widget.h"
#include "ui_project_widget.h"

ProjectWidget::ProjectWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ProjectWidget)
{
    ui->setupUi(this);
}

ProjectWidget::~ProjectWidget()
{
    delete ui;
}
