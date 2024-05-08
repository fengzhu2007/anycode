#ifndef SFTPWORKER_H
#define SFTPWORKER_H
#include <QString>
#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <QVariant>
#include "local/FileItem.h"
namespace ady {
    class SFTP;
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





    class SFTPWorkerTask{
    public:
        SFTPWorkerTask();
        SFTPWorkerTask(QString cmd,QVariant data);
        ~SFTPWorkerTask();
        QString cmd;
        QVariant data;
    };

    class SFTPWorker : public QThread
    {
        Q_OBJECT
    public:
        constexpr const static char W_LINK[] = "link";
        constexpr const static char W_CHDIR[] = "chdir";
        constexpr const static char W_UNLINK[] = "unlink";
        //constexpr const static char W_PASV[] = "pasv";
        constexpr const static char W_MKDIR[] = "mkdir";
        constexpr const static char W_RENAME[] = "rename";
        constexpr const static char W_CHMOD[] = "chmod";
        constexpr const static char W_DELETE[] = "delete";
        constexpr const static char W_ERROR[] = "error";

    public:
        SFTPWorker(long long siteid , QObject* parent=nullptr);
        ~SFTPWorker();
        SFTPWorkerTask* take();
        virtual void run() override;
        void append(SFTPWorkerTask* task);
        void append(QString cmd,QVariant data);
        void stop();

    /*public slots:
        void link(SFTP* sftp);
        void chDir(SFTP* sftp,QString dir);
        void unlink(SFTP* sftp);*/

    signals:
        void taskFinished(QString name,NetworkResponse* response);


    private:
        void chmod(SFTP*sftp,QString dst,int mode,int type,bool applyChildren,int& fileCont);
        void del(SFTP*sftp,QString dst,int type,int& fileCount,int&folderCount);

    private:
        long long m_siteid;
        QList<SFTPWorkerTask*> m_tasks;
        QWaitCondition* m_cond;
        QMutex* m_mutex;
        bool m_is_starting;


    };


}
#endif // SFTPWORKER_H
