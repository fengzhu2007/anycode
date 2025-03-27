#ifndef DBMS_THREAD_H
#define DBMS_THREAD_H
#include "network/network_response.h"
#include "network/network_request.h"
#include <QThread>
namespace ady{

class DBMSThreadPrivate;
class DBMSThread : public QThread
{
    Q_OBJECT
public:
    enum Command{
        Link=0,
        Query,
        Unlink
    };
    enum Result{
        Success=0,
        Failed,
        Unsppport,
        Interrupt
    };

    DBMSThread(int command,void* data=nullptr,QObject *parent = nullptr);
    ~DBMSThread();
    //void interrupt();

protected:
    virtual void run() override;

private:


private:
    DBMSThreadPrivate* d;

signals:
    void resultReady(NetworkResponse* response,int command,int result=0);
    void message(const QString& message,int result);
    void output(const QString& message,int status);
    //void output(NetworkResponse* response);

};
}

#endif // DBMS_THREAD_H
