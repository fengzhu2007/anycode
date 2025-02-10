#include "ssl_query_task.h"
#include "ssl_querier.h"

namespace ady{

SSLQueryTask::SSLQueryTask(const QStringList& list):ScheduleTask(0),m_sitelist(list) {


}

void SSLQueryTask::execute(){
    SSLQuerier query(m_sitelist);
    bool ret = query.execute();
}

SSLQueryTask::~SSLQueryTask(){

}

}
