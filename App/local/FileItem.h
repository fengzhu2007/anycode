#ifndef _FILEITEM_
#define _FILEITEM_
#include "../global.h"
#include <QString>

namespace ady {
    class ANYENGINE_EXPORT FileItem
    {
    public:
        enum Type{
            Unkown=0,
            File=1,
            Dir=2
        };



        FileItem();

    public:
        QString name;
        QString path;
        QString permission;
        QString ext;
        qint64 size;//file size;
        QString modifyTime;
        QString owner;
        QString group;
        Type type;
        bool enabled;
        bool match_path;


    };
}



#endif