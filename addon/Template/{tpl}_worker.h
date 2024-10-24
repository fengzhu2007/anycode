#ifndef {TPL}WORKER_H
#define {TPL}WORKER_H
#include <QString>
#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <QVariant>
#include "local/FileItem.h"
namespace ady {
    class {TPL};
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





    class {TPL}WorkerTask{
    public:
        {TPL}WorkerTask();
        {TPL}WorkerTask(QString cmd,QVariant data);
        ~{TPL}WorkerTask();
        QString cmd;
        QVariant data;
    };

    class {TPL}Worker : public QThread
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
        {TPL}Worker(long long siteid , QObject* parent=nullptr);
        ~{TPL}Worker();
        {TPL}WorkerTask* take();
        virtual void run() override;
        void append({TPL}WorkerTask* task);
        void append(QString cmd,QVariant data);
        void stop();

    /*public slots:
        void link({TPL}* {tpl});
        void chDir({TPL}* {tpl},QString dir);
        void unlink({TPL}* {tpl});*/

    signals:
        void taskFinished(QString name,NetworkResponse* response);


    private:
        void chmod({TPL}*{tpl},QString dst,int mode,int type,bool applyChildren,int& fileCont);
        void del({TPL}*{tpl},QString dst,int type,int& fileCount,int&folderCount);

    private:
        long long m_siteid;
        QList<{TPL}WorkerTask*> m_tasks;
        QWaitCondition* m_cond;
        QMutex* m_mutex;
        bool m_is_starting;


    };


}
#endif // {TPL}WORKER_H
