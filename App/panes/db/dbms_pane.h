#ifndef DBMS_PANE_H
#define DBMS_PANE_H

#include "global.h"
#include <docking_pane.h>
#include "core/event_bus/subscriber.h"

namespace Ui {
class DBMSPane;
}
namespace ady{

class DBDriver;
class DBRecord;
class TablePane;
using InitDBConnectFunc = std::function<DBDriver*(const DBRecord&)>;

class DBMSPanePrivate;
class ANYENGINE_EXPORT DBMSPane : public DockingPane , public Subscriber
{
    Q_OBJECT
public:
    ~DBMSPane();
    void initView();
    virtual QString id() override;
    virtual QString group() override;
    virtual bool onReceive(Event* e) override;//event bus receive callback

    //void addThread(ServerRequestThread* thread);

    static DBMSPane* open(DockingPaneManager* dockingManager,bool active=false);
    static DBMSPane* make(DockingPaneManager* dockingManager,const QJsonObject& data);
    static DBMSPane* getInstance();


    template <typename T>
    void registerInitCallback(const QString& driver,const QString& title){
        m_initConnectList[{driver,title}] = [](const DBRecord& data){
            return new T(data);
        };
    }

    void openConnectDialog(const QString& driver,const DBRecord& data);

    TablePane* openTablePane(long long id,const QString& table);

    DBDriver* connector(long long id);



public slots:
    void onContextMenu(const QPoint& pos);





    void connectServer(long long id,const QString& path);

    //void onNetworkResponse(NetworkResponse* response,int command,int result=0);
    void onMessage(const QString& message,int result);
    void onTreeItemExpanded(const QModelIndex& index);
    void onTreeItemDClicked(const QModelIndex& index);
    void onActionTriggered();
    //void onRename(ServerManageModelItem* item,const QString& newName);
    void onThreadFinished();
    void onOutput(const QString& message,int status);
    //void onDropUpload(const QMimeData* data);



private:
    DBMSPane(QWidget *parent = nullptr);

    DBDriver* openConnect(const DBRecord& data);
    //void chmod(int mode,bool apply_children);
    //void refresh(ServerManageModelItem* item);
    //void output(NetworkResponse* response);

public:
    static const QString PANE_ID;
    static const QString PANE_GROUP;
    //static const QString PANE_TITLE;

private:
    Ui::DBMSPane *ui;
    DBMSPanePrivate* d;
    static DBMSPane* instance;
    std::map<QPair<QString,QString>,InitDBConnectFunc> m_initConnectList;


};

}

#endif // DBMS_PANE_H
