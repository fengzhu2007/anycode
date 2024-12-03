#ifndef AUTO_SAVE_TASK_H
#define AUTO_SAVE_TASK_H

#include "schedule_task.h"
namespace ady{
class AutoSaveTask : public ScheduleTask
{
public:
    AutoSaveTask(int msec);
    virtual void execute() override;
    virtual void doing() override;
};
}
#endif // AUTO_SAVE_TASK_H
