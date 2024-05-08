#ifndef TASKTHREAD_H
#define TASKTHREAD_H
#include "global.h"
#include <QThread>
#include <QList>
namespace ady {
    class Task;
    class ANYENGINE_EXPORT TaskThread : public QThread
    {
        Q_OBJECT
    public:
        TaskThread(long long siteid,QObject* parent=nullptr);
        virtual void run() override;
        inline long long siteId(){return m_siteid;}

    signals:
        void removeTask(Task*);
        void failTask(Task*);
        void insertTasks(QList<Task*>);

    private:
        QList<Task*> travelLocalDir(Task* task);
        QList<Task*> travelRemoteDir(Task* task);

    private:
        long long m_siteid;

    };
}
#endif // TASKTHREAD_H
