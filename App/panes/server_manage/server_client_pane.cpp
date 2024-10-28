#include "server_client_pane.h"
#include "interface/panel.h"

namespace ady{

const QString ServerClientPane::PANE_ID = "ServerClient_%1_%2";
const QString ServerClientPane::PANE_GROUP = "ServerClient";

class ServerClientPanePrivate{
public:
    QString type;
    long long id=0l;
    Panel* panel;
};

ServerClientPane::ServerClientPane(QWidget* parent,long long id):
 DockingPane(parent)
{
    Subscriber::reg();
    d = new ServerClientPanePrivate;
    d->id = id;
}


ServerClientPane::~ServerClientPane(){
    Subscriber::unReg();
    delete d;
}



void ServerClientPane::initView(){

}


void ServerClientPane::setCenterWidget(Panel* widget){
    d->panel = widget;
    DockingPane::setCenterWidget(widget);
}



QString ServerClientPane::id(){

    return PANE_ID.arg(d->type).arg(d->id);
}


QString ServerClientPane::group(){
    return PANE_GROUP;
}


bool ServerClientPane::onReceive(Event* e){

    return false;

}


}
