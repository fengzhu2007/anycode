#ifndef SCHEDULE_H
#define SCHEDULE_H
#include <QObject>
namespace ady{
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

    void addAutoSave(int msec);

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
