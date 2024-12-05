#include "network_auto_close_task.h"
#include "network/network_manager.h"

namespace ady{

NetworkAutoCloseTask::NetworkAutoCloseTask(int msec):ScheduleTask(msec) {

}


void NetworkAutoCloseTask::execute(){
    auto instance = NetworkManager::getInstance();
    if(instance){
        instance->autoClose();
    }
}

}
