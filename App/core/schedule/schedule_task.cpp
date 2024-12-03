#include "schedule_task.h"
#include <QDateTime>
namespace ady{
ScheduleTask::ScheduleTask(int msec):m_interval(msec) {

}

void ScheduleTask::doing(){


}

void ScheduleTask::done() {
    m_last_execution_time = QDateTime::currentDateTime().toMSecsSinceEpoch();
}

}
