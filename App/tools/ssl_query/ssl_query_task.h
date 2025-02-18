#ifndef SSL_QUERY_TASK_H
#define SSL_QUERY_TASK_H
#include "core/schedule/schedule_task.h"
#include <QString>
#include <QJsonObject>
#include <QStringList>

namespace ady{

class SSLQueryDialog;
class SSLQuerier;
class SSLQueryTask :  public ScheduleTask
{
public:
    SSLQueryTask(const QStringList& list,long long delay=0);
    virtual void execute() override;
    ~SSLQueryTask() override;
    inline SSLQuerier* querier(){
        return m_querier;
    }

public:
    static long long DELAY;
    static QString SSL_QUERIER_KEY;

private:
    QStringList m_sitelist;
    SSLQuerier* m_querier;
    friend class Schedule;
};

}
#endif // SSL_QUERY_TASK_H
