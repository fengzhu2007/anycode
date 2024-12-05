#include "network_status_task.h"
#include "network/network_request.h"
#include "network/network_listen.h"
namespace ady{

NetworkStatusTask::NetworkStatusTask(int msec):ScheduleTask(msec) {

    m_listen = new NetworkListen();
}



void NetworkStatusTask::execute(){

}

}
