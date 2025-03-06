#include "ssl_query_task.h"
#include "ssl_querier.h"
#include <QtConcurrent>
namespace ady{

#ifdef Q_DEBUG
long long SSLQueryTask::DELAY = 5 * 1000;//delay 1 hour
#else
long long SSLQueryTask::DELAY = 1 * 3600 * 1000;//delay 1 hour
#endif

QString SSLQueryTask::SSL_QUERIER_KEY = QLatin1String("ssl_querier_domains");
//once task
SSLQueryTask::SSLQueryTask(const QStringList& list,long long delay):ScheduleTask(0,delay),m_sitelist(list) {
    this->m_querier = new SSLQuerier(list);
}

void SSLQueryTask::execute(){
    auto querier = this->m_querier;
#ifdef Q_DEBUG
    qDebug()<<"SSLQueryTask::execute";
#endif
    QtConcurrent::run([querier](){
#ifdef Q_DEBUG
        qDebug()<<"run";
#endif
        querier->execute();
    });
}

SSLQueryTask::~SSLQueryTask(){

}


}
