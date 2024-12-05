#ifndef NETWORK_RATE_TASK_H
#define NETWORK_RATE_TASK_H

#include "schedule_task.h"
namespace ady{
class Schedule;
class NetworkRateTask : public ScheduleTask
{
public:
    virtual void execute() override;

private:
    NetworkRateTask(int msec);
    friend class Schedule;
};
}

#endif // NETWORK_RATE_TASK_H
