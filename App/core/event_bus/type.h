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
    static const QString M_CLOSE_PROJECT;
    static const QString M_CLOSE_PROJECT_NOTIFY;
    static const QString M_FILE_CHANGED;
    static const QString M_OPEN_EDITOR;
    static const QString M_OPEN_PANE;

    //editor manage
    static const QString M_WILL_RENAME_FILE;
    static const QString M_WILL_RENAME_FOLDER ;
    static const QString M_RENAMED_FILE;
    static const QString M_RENAMED_FOLDER;
    static const QString M_RELOAD_FILE;
    static const QString M_DELETE_FILE;
    static const QString M_DELETE_FOLDER;


    //version control event message list id
    static const QString M_OPEN_FILE_TRANSFTER;
    static const QString M_UPLOAD;
    static const QString M_QUERY_COMMIT;
    static const QString M_QUERY_DIFF;

    static const QString M_DOWNLOAD;

    static const QString M_FIND;

    static const QString M_OPEN_FIND;
    static const QString M_GOTO;


    //server manager
    static const QString M_NOTIFY_REFRESH_TREE;
    static const QString M_NOTIFY_REFRESH_LIST;


    //project
    static const QString M_SITE_ADDED;
    static const QString M_SITE_UPDATED;
    static const QString M_SITE_DELETED;

    //statusbar
    static const QString M_MESSAGE;
    static const QString M_READY;

    //ssl query
    static const QString M_SSL_RESULT;
    static const QString M_SSL_ERROR;

    //window
    static const QString M_RESTART;

    //terminal
    static const QString M_OPEN_TERMINAL;

    //notification
    static const QString M_NOTIFICATION;
    static const QString M_TOGGLE_NOTIFICATION;

    static const QString M_RESOURCE_LOCATION;

    static const QString M_SHOW_PROJECT;

    //database
    static const QString M_NEW_CONNECTION;
    static const QString M_UPDATE_CONNECTION;

    //output
    enum OutputLevel{
        Text=0,
        Ok,
        Warning,
        Error
    };
    static const QString M_OUTPUT;
};
}

#endif // TYPE_H
