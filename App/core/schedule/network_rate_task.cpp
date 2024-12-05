#include "network_rate_task.h"
#include "panes/file_transfer/file_transfer_model.h"
#include "components/statusbar/status_bar_view.h"


namespace ady{

NetworkRateTask::NetworkRateTask(int msec):ScheduleTask(msec) {


}

void NetworkRateTask::execute(){
    auto instance = FileTransferModel::getInstance();
    if(instance!=nullptr){
        auto rate = instance->rate();
        auto statusBar = StatusBarView::getInstance();
        if(statusBar){
            statusBar->setRate(rate);
        }
    }
}


}
