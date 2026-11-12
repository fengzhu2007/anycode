#ifndef TASK_H
#define TASK_H
#include "global.h"
#include <QString>
#include <QMap>
#include <QVariant>
#include <QFile>
#include <atomic>
namespace ady {
class FileTransferModelItem;
    class ANYENGINE_EXPORT Task {
    public:

        constexpr const static char MODE[] = "mode";
        constexpr const static char APPLY_CHILDREN[] = "apply_children";


        Task();
        ~Task();
        Task(long long siteid,QString local,QString remote);
        Task(FileTransferModelItem* item);
    public:
        char cmd;
        char type;
        //char direction;
        QString local;
        QString remote;
        long long siteid;
        long long id;//task id
        long long filesize;
        long long readysize;
        QFile* file;
        QMap<QString,QVariant> data;
        char status;
        QString errorMsg;
        //中止标记会被传输线程轮询,必须使用原子类型避免数据竞争
        std::atomic<bool> abort;

        //进度上报节流状态,只由当前传输线程读写
        long long notify_time = 0;//上次上报时间(ms)
        long long notify_size = 0;//上次上报时已传输的字节数
        float notify_progress = -1.0f;//上次上报的进度



        static long long seq ;
    };
}
#endif // TASK_H
