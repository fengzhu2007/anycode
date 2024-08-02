#ifndef BACKENDTHREAD_H
#define BACKENDTHREAD_H
#include "global.h"
#include <QThread>

namespace ady{
class BackendThreadTaskPrivate;
class BackendThreadTask{
public:
    enum Type{
        ReadFolder=0,
        ReadFile,
        RefreshFolder,
        ReadFolderAndInsertFile,
        ReadFolderAndInsertFolder,
        QueryCommit,
        QueryDiff,

        Custome
    };
    BackendThreadTask(int type);
    BackendThreadTask(int type,void* data);
    virtual ~BackendThreadTask();
    virtual bool exec()=0;
    int type();
    void* data();
    void setType(int type);

private:
    BackendThreadTaskPrivate* d;
};


class BackendThreadPrivate;
    class ANYENGINE_EXPORT BackendThread : public QThread
    {
        Q_OBJECT
    public:
        static BackendThread* getInstance();
        static BackendThread* init();
        BackendThread* stop();

        ~BackendThread();

        void appendTask(BackendThreadTask* task);
        //void appendTask(BackendThreadTask::Type,void* data);
        virtual void run() override;

    private:
        explicit BackendThread(QObject *parent = nullptr);
        BackendThreadTask* takeFirst();

        //do task
        /*void doReadFolder(bool refresh=false);
        void doReadFile();
        void doQueryCommit();
        void doQueryDiff();*/

    private:
        BackendThreadPrivate* d;
        static BackendThread* instance;
    };
}

#endif // BACKENDTHREAD_H
