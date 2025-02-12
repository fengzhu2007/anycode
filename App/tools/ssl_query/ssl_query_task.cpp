#include "ssl_query_task.h"
#include "ssl_querier.h"
#include <QtConcurrent>
namespace ady{

SSLQueryTask::SSLQueryTask(const QStringList& list):ScheduleTask(0),m_sitelist(list) {
    this->m_querier = new SSLQuerier(list);
}

void SSLQueryTask::execute(){
    auto querier = this->m_querier;
    qDebug()<<"execute";
    QtConcurrent::run([querier](){
        qDebug()<<"run";
        querier->execute();
    });
}

SSLQueryTask::~SSLQueryTask(){

}

}
