#include "network_status_task.h"
#include "network/network_request.h"
#include "network/network_listen.h"
#include "components/statusbar/status_bar_view.h"
#include <QtConcurrent>
namespace ady{

NetworkStatusTask::NetworkStatusTask(int msec):ScheduleTask(msec) {

    m_listen = new NetworkListen();
    auto instance = StatusBarView::getInstance();
    QObject::connect(m_listen,&NetworkListen::onlineStateChanged,instance,&StatusBarView::setNetworkStatus);

}

NetworkStatusTask::~NetworkStatusTask(){
    delete m_listen;
}



void NetworkStatusTask::execute(){

    QtConcurrent::run([this](){
        m_listen->execute();
    });

}



}
