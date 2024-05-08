#ifndef TASK_H
#define TASK_H
#include "global.h"
#include <QString>
#include <QMap>
#include <QVariant>
#include <QFile>
namespace ady {
    class ANYENGINE_EXPORT Task {
    public:

        constexpr const static char MODE[] = "mode";
        constexpr const static char APPLY_CHILDREN[] = "apply_children";


        Task();
        Task(long long siteid,QString local,QString remote);
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
        bool abort;



        static long long seq ;
    };
}
#endif // TASK_H
