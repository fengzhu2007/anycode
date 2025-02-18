#ifndef SCHEDULE_H
#define SCHEDULE_H
#include <QObject>
namespace ady{
class ScheduleTask;
class SchedulePrivate;
class Schedule : public QObject
{
    Q_OBJECT
public:
    ~Schedule();
    static Schedule * getInstance();
    static void init(QObject* parent);
    static void stop();
    static void start();

    void addFileAutoSave(int msec);
    void addNetworkAutoClose(int msec=300 * 1000);
    void addNetworkStatusWatching(int msec=2 * 1000);

    void addSSLQuery();

    void addTask(ScheduleTask* task);

public slots:
    void onTimeout();

private:
    Schedule(QObject* parent);

private:
    static Schedule* instance;
    SchedulePrivate* d;
};
}
#endif // SCHEDULE_H
