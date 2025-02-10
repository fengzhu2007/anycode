#ifndef SCHEDULE_TASK_H
#define SCHEDULE_TASK_H
#include "global.h"
namespace ady{
class ANYENGINE_EXPORT ScheduleTask
{
public:
    ScheduleTask(int msec);
    virtual ~ScheduleTask();
    virtual void execute()=0;
    virtual void doing();
    virtual void done();
    inline bool isShouldExecute(long long current_time){
        return current_time > m_last_execution_time + m_interval;
    };
    inline bool isOnce(){
        return m_interval==0;
    }
protected:
    int m_interval;//unit msec
    long long m_last_execution_time;//unit msec
};
}
#endif // SCHEDULE_TASK_H
