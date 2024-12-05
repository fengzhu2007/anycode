#ifndef FILE_AUTO_SAVE_TASK_H
#define FILE_AUTO_SAVE_TASK_H

#include "schedule_task.h"
namespace ady{
class Schedule;
class FileAutoSaveTask : public ScheduleTask
{
public:

    virtual void execute() override;
    virtual void doing() override;

private:
    FileAutoSaveTask(int msec);

    friend class Schedule;
};
}
#endif // FILE_AUTO_SAVE_TASK_H
