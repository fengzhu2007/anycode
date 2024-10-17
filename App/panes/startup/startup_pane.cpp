#include "startup_pane.h"
namespace ady{

const QString StartupPane::PANE_ID = "Startup";
const QString StartupPane::PANE_GROUP = "Startup";

StartupPane::StartupPane(QWidget *parent)
    :DockingPane(parent)
{
    QWidget* widget = new QWidget(this);//keep level like createPane(id,group...)
    widget->setObjectName("widget");
    //ui->setupUi(widget);
    this->setCenterWidget(widget);
    this->setWindowTitle(tr("Startup"));
}


StartupPane::~StartupPane(){

}
QString StartupPane::id() {
    return PANE_ID;
}
QString StartupPane::group() {
    return PANE_GROUP;
}
QString StartupPane::description() {
    return this->windowTitle();
}
}
