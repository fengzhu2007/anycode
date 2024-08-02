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
    void interrupt();

protected:
    virtual void run() override;

private:
    void delFile(const QString& path);
    void delFolder(const QString& path);

private:
    ServerRequestThreadPrivate* d;

signals:
    void resultReady(NetworkResponse* response,int command,int result=0);

};
}

#endif // SERVERREQUESTTHREAD_H
