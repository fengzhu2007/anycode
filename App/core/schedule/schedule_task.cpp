#include "schedule_task.h"
#include <QDateTime>
namespace ady{
ScheduleTask::ScheduleTask(int msec,int delay):m_interval(msec),m_delay(delay) {

}

ScheduleTask::~ScheduleTask(){


}

void ScheduleTask::doing(){


}

void ScheduleTask::done() {
    m_last_execution_time = QDateTime::currentDateTime().toMSecsSinceEpoch();
}

}
