#ifndef NETWORK_STATUS_TASK_H
#define NETWORK_STATUS_TASK_H

#include "schedule_task.h"
namespace ady{
class Schedule;
class NetworkListen;
class NetworkStatusTask : public ScheduleTask
{
public:
    virtual void execute() override;

private:
    NetworkStatusTask(int msec);

private:
    NetworkListen* m_listen;
    friend class Schedule;
};
}


#endif // NETWORK_STATUS_TASK_H
