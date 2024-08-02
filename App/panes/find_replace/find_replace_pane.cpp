#include "find_replace_pane.h"
#include "ui_find_replace_pane.h"

FindReplacePane::FindReplacePane(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FindReplacePane)
{
    ui->setupUi(this);
}

FindReplacePane::~FindReplacePane()
{
    delete ui;
}
