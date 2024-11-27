#include "error_pane.h"
#include "panes/error/ui_error_pane.h"
#include "ui_error_pane.h"
#include <docking_pane_manager.h>

namespace ady{

const QString ErrorPane::PANE_ID = "Error_%1";
const QString ErrorPane::PANE_GROUP = "Error";

static int SN = 1;
class ErrorPanePrivate{
public:
    long long id;
};

ErrorPane::ErrorPane(QWidget *parent)
    : DockingPane(parent)
    , ui(new Ui::ErrorPane)
{
    ui->setupUi(this);
    d = new ErrorPanePrivate;
    d->id = SN++;
}

ErrorPane::~ErrorPane()
{
    delete d;
    delete ui;
}


QString ErrorPane::id(){
    return PANE_ID.arg(d->id);
}
QString ErrorPane::group(){
    return PANE_GROUP;
}

ErrorPane* ErrorPane::make(DockingPaneManager* dockingManager,const QJsonObject& data){
    auto pane = new ErrorPane(dockingManager->widget());
    return pane;
}

}
