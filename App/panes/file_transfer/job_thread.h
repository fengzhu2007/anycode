#ifndef JOBTHREAD_H
#define JOBTHREAD_H

#include <QThread>
#include <QList>
#include <QFileInfo>
#include "interface/file_item.h"
namespace ady{
class FileTransferModelItem;
class FileTransferModel;
class Task;
class JobThread : public QThread
{
    Q_OBJECT
public:
    explicit JobThread(long long id,FileTransferModel *parent = nullptr);
    virtual void run() override;

    inline long long id(){return m_siteid;}
    void abort();

signals:
    void finishTask(long long siteid,long long id);
    void errorTask(long long siteid,long long id,const QString& errorMsg);
    void uploadFolder(long long siteid,long long id,QFileInfoList list,int state);
    void donwloadFolder(long long siteid,long long id,QList<FileItem> list,int state);


private:
    QFileInfoList listLocal(const QString& source);
    QList<FileItem> listRemote(long long siteid,const QString& destination,QString* errorMsg);

protected:
    long long m_siteid;
    Task* m_task;

};
}
#endif // JOBTHREAD_H
