#ifndef SERVERCLIENTPANE_H
#define SERVERCLIENTPANE_H
#include "global.h"
#include "docking_pane.h"
#include "core/event_bus/subscriber.h"
#include <QMimeData>
#include <QJsonObject>

namespace Ui {
class ServerClientPane;
}


namespace ady{
class DockingPaneManager;
class NetworkResponse;
class ServerClientPanePrivate;
class ANYENGINE_EXPORT ServerClientPane : public DockingPane  , public Subscriber
{
    Q_OBJECT

public:
    ServerClientPane(QWidget* parent,long long id);

    ~ServerClientPane();
    void initView();
    virtual QString id() override;
    virtual QString group() override;
    virtual bool onReceive(Event* e) override;//event bus receive callback
    virtual QJsonObject toJson() override;

    bool isThreadRunning();

    void setRootPath(const QString& path);
    QString rootPath();
    void setCurrentPath(const QString& path,bool request=false);
    QString currentPath();

    void reload();

    static ServerClientPane* make(DockingPaneManager* dockingManager,const QJsonObject& data);

protected:
    virtual void showEvent(QShowEvent* e) override;

public slots:
    void onContextMenu(const QPoint& pos);
    void connectServer(long long id,const QString& path);
    void cdDir(long long id,const QString& path,bool refresh=false);
    void deleteFiles(long long id,const QStringList& list);
    void onNetworkResponse(NetworkResponse* response,int command,int result=0);
    void onMessage(const QString& message,int result);
    //void onTreeItemExpanded(const QModelIndex& index);
    void onTreeItemDClicked(const QModelIndex& index);
    void onDropUpload(const QMimeData* data);
    void onActionTriggered();
    void onNewFolder(long long id,const QString& path,const QString& name);
    void onRename(const QModelIndex index,const QString& newName);
    void onThreadFinished();



public:
    static const QString PANE_ID;
    static const QString PANE_GROUP;

private:
    ServerClientPanePrivate* d;
    Ui::ServerClientPane* ui;


};
}
#endif // SERVERCLIENTPANE_H
