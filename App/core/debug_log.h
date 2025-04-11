#ifndef DEBUG_LOG_H
#define DEBUG_LOG_H
#include <QString>

namespace ady{
class DebugLogPrivate;
class DebugLog
{
public:
    ~DebugLog();
    static DebugLog* getInstance();
    static void destory();
    static void write(const QString& category,const QString& content);
private:
    DebugLog();
private:
    static DebugLog* instance;
    DebugLogPrivate* d;
};
}
#endif // DEBUG_LOG_H
