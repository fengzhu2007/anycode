#ifndef JOBTHREAD_H
#define JOBTHREAD_H

#include <QThread>
#include <QList>
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

private:
    QList<FileTransferModelItem*> listRemote(FileTransferModelItem* parent);
    QList<FileTransferModelItem*> listLocal(FileTransferModelItem* parent);

protected:
    long long m_siteid;
    Task* m_task;

};
}
#endif // JOBTHREAD_H
