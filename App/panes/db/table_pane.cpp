#include "table_pane.h"
#include "ui_table_pane.h"



namespace ady{


const QString TablePane::PANE_ID = "DBTable_%1";
const QString TablePane::PANE_GROUP = "DBTable";
int TablePane::SN = 0;


class TablePanePrivate{
public:
    int id;
    QString name;//table name

};

TablePane::TablePane(QWidget *parent)
    : DockingPane(parent)
    , ui(new Ui::TablePane)
{
    d = new TablePanePrivate;
    d->id = TablePane::SN;
    TablePane::SN+=1;

    QWidget* widget = new QWidget(this);//keep level like createPane(id,group...)
    widget->setObjectName("widget");
    ui->setupUi(widget);

}

TablePane::~TablePane()
{
    delete ui;
}

void TablePane::initView(){

}

QString TablePane::id(){
    return PANE_ID.arg(d->id);
}

QString TablePane::group(){
    return PANE_GROUP;
}




}
