#ifndef RESOURCE_MANAGER_PANE_H
#define RESOURCE_MANAGER_PANE_H
#include "global.h"
#include "docking_pane.h"
#include "core/event_bus/subscriber.h"

namespace Ui {
class ResourceManagerPane;
}

namespace ady{
class DockingPaneManager;
class ResourceManagerModelItem;
class ResourceManagerPanePrivate;
class ANYENGINE_EXPORT ResourceManagerPane : public DockingPane , public Subscriber
{
    Q_OBJECT

public:

    ~ResourceManagerPane();
    void initView();
    virtual QString id() override;
    virtual QString group() override;
    virtual bool onReceive(Event* e) override;//event bus receive callback
    void readFolder(ResourceManagerModelItem* item);

    static ResourceManagerPane* open(DockingPaneManager* dockingManager,bool active=false);

public slots:
    void onTreeItemExpanded(const QModelIndex& index);
    void onTreeItemDClicked(const QModelIndex& index);
    void showContextMenu(const QPoint& pos);
    void onActionTriggered();

private:
    ResourceManagerPane(QWidget *parent = nullptr);

public:
    //event message list id
    static const QString M_OPEN_PROJECT;
    static const QString M_FILE_CHANGED;
    static const QString M_OPEN_EDITOR;

    static const QString PANE_ID;
    static const QString PANE_GROUP;

private:
    Ui::ResourceManagerPane *ui;
    ResourceManagerPanePrivate* d;
    static ResourceManagerPane* instance;




};

}
#endif // RESOURCE_MANAGER_PANE_H
