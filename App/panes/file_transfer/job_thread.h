#ifndef JOBTHREAD_H
#define JOBTHREAD_H

#include <QThread>
#include <QList>
namespace ady{
class FileTransferModelItem;
class FileTransferModel;
class JobThread : public QThread
{
    Q_OBJECT
public:
    explicit JobThread(long long id,FileTransferModel *parent = nullptr);
    virtual void run() override;

private:
    QList<FileTransferModelItem*> listRemote(FileTransferModelItem* parent);
    QList<FileTransferModelItem*> listLocal(FileTransferModelItem* parent);

protected:
    long long m_siteid;

};
}
#endif // JOBTHREAD_H
