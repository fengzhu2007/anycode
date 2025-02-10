#ifndef SSL_QUERY_TASK_H
#define SSL_QUERY_TASK_H
#include "core/schedule/schedule_task.h"
#include <QString>
#include <QStringList>

namespace ady{

class SSLQueryTask : public ScheduleTask
{
public:
    SSLQueryTask(const QStringList& list);
    virtual void execute() override;
    ~SSLQueryTask() override;

private:
    QStringList m_sitelist;

    friend class Schedule;
};

}
#endif // SSL_QUERY_TASK_H
