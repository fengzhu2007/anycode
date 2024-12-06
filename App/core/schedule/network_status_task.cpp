#include "network_status_task.h"
#include "network/network_request.h"
#include "network/network_listen.h"
#include <QtConcurrent>
namespace ady{

NetworkStatusTask::NetworkStatusTask(int msec):ScheduleTask(msec) {

    m_listen = new NetworkListen();

}



void NetworkStatusTask::execute(){
    QtConcurrent::run([this](){
        m_listen->execute();
    });
}

}
