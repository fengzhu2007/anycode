#include "quick_pane.h"
#include <docking_pane_manager.h>
#include <docking_pane_layout_item_info.h>
#include <QQuickWidget>
#include <QtQuickControls2>


namespace ady{

QuickPane* QuickPane::instance = nullptr;

const QString QuickPane::PANE_ID = "Quick";
const QString QuickPane::PANE_GROUP = "Quick";


class QuickPanePrivate{
public:
    QQuickWidget* widget;

};

QuickPane::QuickPane(const QString qml,QWidget* parent):DockingPane(parent) {
    //QQuickStyle::setStyle("Material");
    d = new QuickPanePrivate;
    d->widget = new QQuickWidget(this);
    d->widget->setSource(QUrl::fromLocalFile(qml));
    this->setWindowTitle(tr("Quick Window"));
    this->setCenterWidget(d->widget);
}

QuickPane::~QuickPane(){
    delete d;
}

QString QuickPane::id() {
    return PANE_ID;
}

QString QuickPane::group() {
    return PANE_GROUP;
}

QString QuickPane::description() {
    return tr("description");
}

void QuickPane::activation() {

}

void QuickPane::save(bool rename) {

}

void QuickPane::contextMenu(const QPoint& pos) {

}

QJsonObject QuickPane::toJson() {
    return {};
}

bool QuickPane::closeEnable(){
    return true;
}

void QuickPane::doAction(int a) {

}

QuickPane* QuickPane::open(DockingPaneManager* dockingManager,bool active){
    if(instance==nullptr){
        instance = new QuickPane(":/Resource/extension/pane.qml",dockingManager->widget());
        DockingPaneLayoutItemInfo* item = dockingManager->createPane(instance,DockingPaneManager::Bottom,active);
        item->setManualSize(220);
    }
    return instance;
}

QuickPane* QuickPane::make(DockingPaneManager* dockingManager,const QJsonObject& data){
    if(instance==nullptr){
        instance = new QuickPane(":/Resource/extension/pane.qml",dockingManager->widget());
        return instance;
    }
    return nullptr;
}

}
