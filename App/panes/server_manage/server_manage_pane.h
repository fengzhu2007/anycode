#ifndef SERVER_MANAGE_PANE_H
#define SERVER_MANAGE_PANE_H
#include "global.h"
#include "docking_pane.h"
#include "core/event_bus/subscriber.h"
#include <QMimeData>


namespace Ui {
class ServerManagePane;
}

namespace ady{
class NetworkResponse;
class ServerRequestThread;
class ServerManageModelItem;
class ServerManagePanePrivate;
class ANYENGINE_EXPORT ServerManagePane : public DockingPane , public Subscriber
{
    Q_OBJECT

public:
    ~ServerManagePane();
    void initView();
    virtual QString id() override;
    virtual QString group() override;
    virtual bool onReceive(Event* e) override;//event bus receive callback

    void addThread(ServerRequestThread* thread);

    static ServerManagePane* open(DockingPaneManager* dockingManager,bool active=false);
    static ServerManagePane* make(DockingPaneManager* dockingManager,const QJsonObject& data);
public slots:
    void onContextMenu(const QPoint& pos);
    void connectServer(long long id,const QString& path);
    void cdDir(long long id,const QString& path,bool refresh=false);
    void deleteFiles(long long id,const QStringList& list);
    void onNetworkResponse(NetworkResponse* response,int command,int result=0);
    void onMessage(const QString& message,int result);
    void onTreeItemExpanded(const QModelIndex& index);
    void onTreeItemDClicked(const QModelIndex& index);
    void onActionTriggered();
    void onNewFolder(long long id,const QString& path,const QString& name);
    void onRename(ServerManageModelItem* item,const QString& newName);
    void onThreadFinished();
    void onOutput(const QString& message,int status);
    void onDropUpload(const QMimeData* data);


private:
    ServerManagePane(QWidget *parent = nullptr);
    void chmod(int mode,bool apply_children);
    void refresh(ServerManageModelItem* item);
    void output(NetworkResponse* response);

public:
    static const QString PANE_ID;
    static const QString PANE_GROUP;
    //static const QString PANE_TITLE;

private:
    Ui::ServerManagePane *ui;
    ServerManagePanePrivate* d;
    static ServerManagePane* instance;
};
}
#endif // SERVER_MANAGE_PANE_H
