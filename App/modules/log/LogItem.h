#ifndef LOGITEM_H
#define LOGITEM_H
#include <QString>
namespace ady {
    class LogItem{
    public:
        enum CMD {
            UPLOAD=1,
            DOWNLOAD,
            DELETE,
            CHMOD
        };
        CMD command;
        QString site_name;
        QString datetime;
        QString description;
    };
}
#endif // LOGITEM_H
