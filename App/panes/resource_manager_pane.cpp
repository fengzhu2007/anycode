#include "resource_manager_pane.h"
#include "ui_resource_manager_pane.h"

const QString PANE_ID = "ResourceManager";
const QString PANE_GROUP = "ResourceManager";
const QString PANE_TITLE = QObject::tr("Resource Manager");

namespace ady{
ResourceManagerPane::ResourceManagerPane(QWidget *parent) :
    DockingPane(parent),
    ui(new Ui::ResourceManagerPane)
{
    //QWidget* widget
    QWidget* widget = new QWidget(this);//keep level like createPane(id,group...)
    ui->setupUi(widget);
    this->setCenterWidget(widget);
}

ResourceManagerPane::~ResourceManagerPane()
{
    delete ui;
}

QString ResourceManagerPane::id(){
    return PANE_ID;
}

QString ResourceManagerPane::group(){
    return PANE_GROUP;

}


}
