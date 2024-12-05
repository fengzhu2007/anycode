#ifndef NETWORK_AUTO_CLOSE_TASK_H
#define NETWORK_AUTO_CLOSE_TASK_H


#include "schedule_task.h"
namespace ady{
class Schedule;
class NetworkAutoCloseTask : public ScheduleTask
{
public:

    virtual void execute() override;

private:
    NetworkAutoCloseTask(int msec);
    friend class Schedule;
};
}

#endif // NETWORK_AUTO_CLOSE_TASK_H
