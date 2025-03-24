#include "dbms_pane.h"
#include "ui_dbms_pane.h"

DBMSPane::DBMSPane(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DBMSPane)
{
    ui->setupUi(this);
}

DBMSPane::~DBMSPane()
{
    delete ui;
}
