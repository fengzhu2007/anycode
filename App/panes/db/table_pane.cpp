#include "table_pane.h"
#include "ui_table_pane.h"
#include "table_list_widget.h"
#include "table_scheme_widget.h"
//#include "dbms_model.h"

namespace ady{


const QString TablePane::PANE_ID = "DBTable_%1";
const QString TablePane::PANE_GROUP = "DBTable";
int TablePane::SN = 0;


class TablePanePrivate{
public:
    int id;
    long long dbid;
    int type;
    QString name;//table name
    TableListWidget* listTab;
    TableSchemeWidget* schemeTab;


};

TablePane::TablePane(long long dbid,int type,const QString& name,QWidget *parent)
    : DockingPane(parent)
    , ui(new Ui::TablePane)
{
    d = new TablePanePrivate;
    d->id = TablePane::SN;
    d->dbid = dbid;
    d->type = type;
    d->name = name;
    TablePane::SN+=1;

    QWidget* widget = new QWidget(this);//keep level like createPane(id,group...)
    widget->setObjectName("widget");
    ui->setupUi(widget);
    this->setCenterWidget(widget);
    this->setStyleSheet("QToolBar{border:0px;}"
                        ".QTabWidget{border:0;}");
    if(name.isEmpty()){
        this->setWindowTitle(tr("New Table"));
    }else{
        this->setWindowTitle(tr("%1").arg(name));
    }

    d->listTab = new TableListWidget(dbid,name,ui->tabWidget);
    d->schemeTab = new TableSchemeWidget(dbid,name,ui->tabWidget);
    ui->tabWidget->addTab(d->listTab,tr("Data"));
    ui->tabWidget->addTab(d->schemeTab,tr("Scheme"));

}

TablePane::~TablePane()
{
    delete ui;
    delete d;
}

void TablePane::initView(){

}

QString TablePane::id(){
    return PANE_ID.arg(d->id);
}

QString TablePane::group(){
    return PANE_GROUP;
}

QString& TablePane::name() const{
    return d->name;
}



}
