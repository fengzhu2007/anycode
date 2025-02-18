#include "ssl_query_task.h"
#include "ssl_querier.h"
#include <QtConcurrent>
namespace ady{

//long long SSLQueryTask::DELAY = 1 * 3600 * 1000;//delay 1 hour
long long SSLQueryTask::DELAY = 5 * 1000;//for test 5 second
QString SSLQueryTask::SSL_QUERIER_KEY = QLatin1String("ssl_querier_domains");
//once task
SSLQueryTask::SSLQueryTask(const QStringList& list,long long delay):ScheduleTask(0,delay),m_sitelist(list) {
    this->m_querier = new SSLQuerier(list);
}

void SSLQueryTask::execute(){
    auto querier = this->m_querier;
    qDebug()<<"SSLQueryTask::execute";
    QtConcurrent::run([querier](){
        //qDebug()<<"run";
        querier->execute();
    });
}

SSLQueryTask::~SSLQueryTask(){

}


}
