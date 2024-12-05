#include "type.h"

namespace ady{
const QString Type::M_OPEN_PROJECT = "Open_Project";
const QString Type::M_CLOSE_PROJECT = "Close_Project";//from menu
const QString Type::M_CLOSE_PROJECT_NOTIFY = "Close_Project_Notify";//after project closed send
const QString Type::M_FILE_CHANGED = "File_Changed";
const QString Type::M_OPEN_EDITOR = "Open_Editor";
const QString Type::M_OPEN_PANE = "OpenPane";

const QString Type::M_WILL_RENAME_FILE = "WillRenameFile";
const QString Type::M_WILL_RENAME_FOLDER = "WillRenameFolder";
const QString Type::M_RENAMED_FILE = "RenamedFile";
const QString Type::M_RENAMED_FOLDER = "RenamedFolder";
const QString Type::M_RELOAD_FILE = "ReloadFile";
const QString Type::M_DELETE_FILE = "DeleteFile";
const QString Type::M_DELETE_FOLDER = "DeleteFolder";

const QString Type::M_OPEN_FILE_TRANSFTER = "OpenFileTransfer";

const QString Type::M_UPLOAD = "Upload";
const QString Type::M_QUERY_COMMIT = "QueryCommit";
const QString Type::M_QUERY_DIFF = "QueryDiff";

const QString Type::M_DOWNLOAD = "Download";

const QString Type::M_FIND = "Find";

const QString Type::M_OPEN_FIND = "OpenFind";

const QString Type::M_GOTO = "Goto";

const QString Type::M_NOTIFY_REFRESH_TREE = "NotifyRefreshTree";
const QString Type::M_NOTIFY_REFRESH_LIST = "NotifyRefreshList";

const QString Type::M_OUTPUT = "Output";

const QString Type::M_SITE_ADDED = "SiteAdded";
const QString Type::M_SITE_UPDATED = "SiteUpdated";
const QString Type::M_SITE_DELETED = "SiteDeleted";


const QString Type::M_MESSAGE = "Message";
const QString Type::M_READY = "Ready";




Type::Type()
{

}



}
