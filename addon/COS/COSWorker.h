#ifndef COSWORKER_H
#define COSWORKER_H
#include <QString>
#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <QVariant>
#include "local/FileItem.h"
namespace ady {
    class COS;
    class NetworkResponse;

    struct ChmodTaskData
    {
        bool applyChildren;
        int mode;
        long long nid;
        QList<FileItem> lists;
    };

    struct DeleteTaskData
    {
        long long nid;
        QList<FileItem> lists;
    };





    class COSWorkerTask{
    public:
        COSWorkerTask();
        COSWorkerTask(QString cmd,QVariant data);
        ~COSWorkerTask();
        QString cmd;
        QVariant data;
    };

    class COSWorker : public QThread
    {
        Q_OBJECT
    public:
        constexpr const static char W_LINK[] = "link";
        constexpr const static char W_CHDIR[] = "chdir";
        constexpr const static char W_UNLINK[] = "unlink";
        //constexpr const static char W_PASV[] = "pasv";
        constexpr const static char W_MKDIR[] = "mkdir";
        //constexpr const static char W_RENAME[] = "rename";
        //constexpr const static char W_CHMOD[] = "chmod";
        constexpr const static char W_DELETE[] = "delete";
        constexpr const static char W_ERROR[] = "error";

    public:
        COSWorker(long long siteid , QObject* parent=nullptr);
        ~COSWorker();
        COSWorkerTask* take();
        virtual void run() override;
        void append(COSWorkerTask* task);
        void append(QString cmd,QVariant data);
        void stop();

    /*public slots:
        void link(COS* cos);
        void chDir(COS* cos,QString dir);
        void unlink(COS* cos);*/

    signals:
        void taskFinished(QString name,NetworkResponse* response);


    private:
        void del(COS* cos,QString bucket,QString dst,int type,int& fileCount,int&folderCount);

    private:
        long long m_siteid;
        QList<COSWorkerTask*> m_tasks;
        QWaitCondition* m_cond;
        QMutex* m_mutex;
        bool m_is_starting;


    };


}
#endif // COSWORKER_H
