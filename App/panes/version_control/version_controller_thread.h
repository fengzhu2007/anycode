#ifndef VERSION_CONTROLLER_THREAD_H
#define VERSION_CONTROLLER_THREAD_H
#include "global.h"
#include <QThread>

namespace ady{
class BackendThreadTask;
class VersionControllerThreadPrivate;

class ANYENGINE_EXPORT VersionControllerThread : public QThread
{
    Q_OBJECT
public:

    explicit VersionControllerThread(BackendThreadTask* task,QObject *parent = nullptr);
    explicit VersionControllerThread(QList<BackendThreadTask*>& tasks,QObject *parent = nullptr);
    ~VersionControllerThread();

    virtual void run() override;

private:


private:
    VersionControllerThreadPrivate* d;
};

}



#endif // VERSION_CONTROLLER_THREAD_H
