#ifndef TYPE_H
#define TYPE_H
#include "global.h"
#include <QString>
namespace ady{
class ANYENGINE_EXPORT Type
{
public:
    Type();
public:
    //resource manage
    static const QString M_OPEN_PROJECT;
    static const QString M_FILE_CHANGED;
    static const QString M_OPEN_EDITOR;

    //editor manage
    static const QString M_WILL_RENAME_FILE;
    static const QString M_WILL_RENAME_FOLDER ;
    static const QString M_RENAMED_FILE;
    static const QString M_RENAMED_FOLDER;
    static const QString M_RELOAD_FILE;
    static const QString M_DELETE_FILE;
    static const QString M_DELETE_FOLDER;


    //version control event message list id
    static const QString M_UPLOAD;
    static const QString M_QUERY_COMMIT;
    static const QString M_QUERY_DIFF;

    static const QString M_DOWNLOAD;
};
}

#endif // TYPE_H