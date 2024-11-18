#ifndef SERVERREQUESTTHREAD_H
#define SERVERREQUESTTHREAD_H
#include "network/network_response.h"
#include "network/network_request.h"
#include <QThread>
namespace ady{
struct RenameData{
    int type;
    QString src;
    QString dest;
    QString parent;
};

struct ChmodData{
    int mode;
    bool apply_children;
    QStringList list;
};


class ServerRequestThreadPrivate;
class ServerRequestThread : public QThread
{
    Q_OBJECT
public:
    enum Command{
        Link=0,
        List,
        Refresh,
        NewFolder,
        Rename,
        Delete,
        Chmod,
        Unlink
    };
    enum Result{
        Success=0,
        Failed,
        Unsppport,
        Interrupt
    };

    ServerRequestThread(NetworkRequest* request,int command,void* data=nullptr,QObject *parent = nullptr);
    ~ServerRequestThread();
    //void interrupt();





protected:
    virtual void run() override;

private:
    bool delFile(const QString& path);
    void delFolder(const QString& path,int* successTotal,int* errorTotal);
    int chmodFile(const QString& path,int);
    void chmodFolder(const QString& path,int mode,bool apply_children,int* successTotal,int* errorTotal);

private:
    ServerRequestThreadPrivate* d;

signals:
    void resultReady(NetworkResponse* response,int command,int result=0);
    void message(const QString& message,int result);
    void output(const QString& message,int status);
    //void output(NetworkResponse* response);

};
}

#endif // SERVERREQUESTTHREAD_H
