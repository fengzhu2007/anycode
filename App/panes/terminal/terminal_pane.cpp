#include "terminal_pane.h"

namespace ady{


TerminalPane* TerminalPane::instance = nullptr;

const QString TerminalPane::PANE_ID = "Terminal";
const QString TerminalPane::PANE_GROUP = "Terminal";


class TerminalPanePrivate{
public:

};



TerminalPane::TerminalPane(QWidget *parent):DockingPane(parent),ui(new Ui::TerminalPane) {

    d = new TerminalPanePrivate;
}


TerminalPane::~TerminalPane(){
    delete ui;
    delete d;
}

void TerminalPane::initView(){

}

QString TerminalPane::id(){
    return TerminalPane::PANE_ID;
}

QString TerminalPane::group(){

    return TerminalPane::PANE_GROUP;
}

bool TerminalPane::onReceive(Event* e) {

}


TerminalPane* TerminalPane::open(DockingPaneManager* dockingManager,bool active){

}

TerminalPane* TerminalPane::make(DockingPaneManager* dockingManager,const QJsonObject& data){

}



}





