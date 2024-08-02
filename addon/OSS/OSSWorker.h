#ifndef OSSWORKER_H
#define OSSWORKER_H
#include <QString>
#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <QVariant>
#include "local/file_item.h"
namespace ady {
    class OSS;
    class NetworkResponse;

    /*struct ChmodTaskData
    {
        bool applyChildren;
        int mode;
        long long nid;
        QList<FileItem> lists;
    };*/

    struct DeleteTaskData
    {
        long long nid;
        QList<FileItem> lists;
    };





    class OSSWorkerTask{
    public:
        OSSWorkerTask();
        OSSWorkerTask(QString cmd,QVariant data);
        ~OSSWorkerTask();
        QString cmd;
        QVariant data;
    };

    class OSSWorker : public QThread
    {
        Q_OBJECT
    public:
        constexpr const static char W_LINK[] = "link";
        constexpr const static char W_CHDIR[] = "chdir";
        constexpr const static char W_UNLINK[] = "unlink";
        constexpr const static char W_PASV[] = "pasv";
        constexpr const static char W_MKDIR[] = "mkdir";
        constexpr const static char W_RENAME[] = "rename";
        constexpr const static char W_DELETE[] = "delete";
        constexpr const static char W_ERROR[] = "error";

    public:
        OSSWorker(long long siteid , QObject* parent=nullptr);
        ~OSSWorker();
        OSSWorkerTask* take();
        virtual void run() override;
        void append(OSSWorkerTask* task);
        void append(QString cmd,QVariant data);
        void stop();

    /*public slots:
        void link(FTP* ftp);
        void chDir(FTP* ftp,QString dir);
        void unlink(FTP* ftp);*/

    signals:
        void taskFinished(QString name,NetworkResponse* response);


    private:
        void del(OSS* oss,QString bucket,QString dst,int type,int& fileCount,int&folderCount);

    private:
        long long m_siteid;
        QList<OSSWorkerTask*> m_tasks;
        QWaitCondition* m_cond;
        QMutex* m_mutex;
        bool m_is_starting;


    };


}
#endif // OSSWORKER_H
