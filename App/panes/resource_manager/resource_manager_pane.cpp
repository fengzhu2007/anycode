#include "resource_manager_pane.h"
#include "ui_resource_manager_pane.h"
#include "resource_manager_model.h"
#include "core/event_bus/event.h"
#include <QDebug>



//const QString PANE_TITLE = QObject::tr("Resource Manager");

namespace ady{

const QString ResourceManagerPane::PANE_ID = "ResourceManager";
const QString ResourceManagerPane::PANE_GROUP = "ResourceManager";

const QString ResourceManagerPane::M_OPEN_PROJECT = "Open_Message";

class ResourceManagerPanePrivate {
public:
    ResourceManagerModel* model;

};

ResourceManagerPane::ResourceManagerPane(QWidget *parent) :
    DockingPane(parent),
    ui(new Ui::ResourceManagerPane)
{
    Subscriber::reg();
    QWidget* widget = new QWidget(this);//keep level like createPane(id,group...)
    ui->setupUi(widget);
    this->setCenterWidget(widget);
    this->setWindowTitle(tr("Resource Manager"));
    this->setStyleSheet("QToolBar{border:0px;}"
                        "QTreeView{border:0}");

    d = new ResourceManagerPanePrivate;
    d->model = new ResourceManagerModel(ui->treeView);
    ui->treeView->setModel(d->model);
}

ResourceManagerPane::~ResourceManagerPane()
{
    Subscriber::unReg();
    delete ui;
    delete d;
}

QString ResourceManagerPane::id(){
    return PANE_ID;
}

QString ResourceManagerPane::group(){
    return PANE_GROUP;

}

bool ResourceManagerPane::onReceive(Event* e){
    if(e->id()==M_OPEN_PROJECT){


        return true;
    }
    return false;
}


}
