#ifndef RESOURCE_MANAGER_PANE_H
#define RESOURCE_MANAGER_PANE_H
#include "global.h"
#include "docking_pane.h"
#include "core/event_bus/subscriber.h"
#include <QMenu>
#include <QMimeData>
namespace Ui {
class ResourceManagerPane;
}


namespace ady {


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
    void readFolder(ResourceManagerModelItem* item,int action=-1);

    static ResourceManagerPane* getInstance();
    static ResourceManagerPane* open(DockingPaneManager* dockingManager,bool active=false);
    static ResourceManagerPane* make(DockingPaneManager* dockingManager,const QJsonObject& data);



public slots:
    void onTreeItemExpanded(const QModelIndex& index);
    void onTreeItemCollapsed(const QModelIndex& index);

    void onTreeItemDClicked(const QModelIndex& index);
    void onContextMenu(const QPoint& pos);
    void onInsertReady(const QModelIndex& ,bool isFile);
    void onItemsChanged(const QString& path);
    void onActionTriggered();
    void onTopActionTriggered();
    void onUploadToSite();
    void onUploadToGroup();
    void onUploadToAll();
    void onSearchFile(const QString& text);
    void onDropAddFolder(const QMimeData* data);
    void onToggleOpend(bool checked);
    void onOpenedSelected(const QModelIndex& index);
    void onLocateSuccess(const QString& path,bool recursion=false);


private:
    ResourceManagerPane(QWidget *parent = nullptr);
    QMenu* attchUploadMenu(QMenu* parent,long long id);
    void uploadSelectedFile(long long siteid);

    void closeProject(ResourceManagerModelItem* item);
    void locationFile();
    void showStatusMessage(long long id,const QString& path);



public:
    static const QString PANE_ID;
    static const QString PANE_GROUP;

private:
    Ui::ResourceManagerPane *ui;
    ResourceManagerPanePrivate* d;
    static ResourceManagerPane* instance;




};

}
#endif // RESOURCE_MANAGER_PANE_H
