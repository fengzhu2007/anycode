#ifndef FTP_WORKER_H
#define FTP_WORKER_H
#include <QString>
#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <QVariant>
#include "local/file_item.h"
namespace ady {
    class FTP;
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





    class FTPWorkerTask{
    public:
        FTPWorkerTask();
        FTPWorkerTask(QString cmd,QVariant data);
        ~FTPWorkerTask();
        QString cmd;
        QVariant data;
    };

    class FTPWorker : public QThread
    {
        Q_OBJECT
    public:
        constexpr const static char W_LINK[] = "link";
        constexpr const static char W_CHDIR[] = "chdir";
        constexpr const static char W_UNLINK[] = "unlink";
        constexpr const static char W_PASV[] = "pasv";
        constexpr const static char W_MKDIR[] = "mkdir";
        constexpr const static char W_RENAME[] = "rename";
        constexpr const static char W_CHMOD[] = "chmod";
        constexpr const static char W_DELETE[] = "delete";
        constexpr const static char W_ERROR[] = "error";

    public:
        FTPWorker(long long siteid , QObject* parent=nullptr);
        ~FTPWorker();
        FTPWorkerTask* take();
        virtual void run() override;
        void append(FTPWorkerTask* task);
        void append(QString cmd,QVariant data);
        void stop();

    /*public slots:
        void link(FTP* ftp);
        void chDir(FTP* ftp,QString dir);
        void unlink(FTP* ftp);*/

    signals:
        void taskFinished(QString name,NetworkResponse* response);


    private:
        void chmod(FTP*ftp,QString dst,int mode,int type,bool applyChildren,int& fileCont);
        void del(FTP*ftp,QString dst,int type,int& fileCount,int&folderCount);

    private:
        long long m_siteid;
        QList<FTPWorkerTask*> m_tasks;
        QWaitCondition* m_cond;
        QMutex* m_mutex;
        bool m_is_starting;


    };


}
#endif // FTP_WORKER_H
