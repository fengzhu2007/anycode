#include "loader.h"
#include "docking_pane.h"
#include "docking_pane_manager.h"
#include "panes/resource_manager/resource_manager_pane.h"
#include "panes/code_editor/code_editor_pane.h"
#include "panes/code_editor/code_editor_manager.h"
#include "panes/version_control/version_control_pane.h"
#include "panes/file_transfer/file_transfer_pane.h"
#include "panes/find_replace/find_replace_pane.h"
#include "panes/server_manage/server_manage_pane.h"
#include "panes/server_manage/server_client_pane.h"
#include "panes/terminal/terminal_pane.h"
#include "panes/output/output_pane.h"
#include "panes/sql/sql_pane.h"
#include "panes/notification/notification_pane.h"
#include "panes/db/dbms_pane.h"

namespace ady{

DockingPane* PaneLoader::init(DockingPaneManager* dockingManager,const QString& group,const QJsonObject& data){
    DockingPane* pane = nullptr;
    if(group == ResourceManagerPane::PANE_GROUP){
        pane = ResourceManagerPane::make(dockingManager,data);
    }else if(group == CodeEditorPane::PANE_GROUP){
        pane = CodeEditorPane::make(dockingManager,data);
    }else if(group == VersionControlPane::PANE_GROUP){
        pane = VersionControlPane::make(dockingManager,data);
    }else if(group == FileTransferPane::PANE_GROUP){
        pane = FileTransferPane::make(dockingManager,data);
    }else if(group == FindReplacePane::PANE_GROUP){
        pane = FindReplacePane::make(dockingManager,data);
    }else if(group == ServerManagePane::PANE_GROUP){
        pane = ServerManagePane::make(dockingManager,data);
    }else if(group == ServerClientPane::PANE_GROUP){
        pane = ServerClientPane::make(dockingManager,data);
    }else if(group == TerminalPane::PANE_GROUP){
        pane = TerminalPane::make(dockingManager,data);
    }else if(group == OutputPane::PANE_GROUP){
        pane = OutputPane::make(dockingManager,data);
    }else if(group == NotificationPane::PANE_GROUP){
        pane = NotificationPane::make(dockingManager,data);
    }else if(group == DBMSPane::PANE_GROUP){
        pane = DBMSPane::make(dockingManager,data);
    }else{
        //other editor
        pane = CodeEditorManager::makePane(group,data);
    }
    return pane;
}

DockingPane* PaneLoader::open(DockingPaneManager* dockingManager,const QString& group,const QJsonObject& data){
    DockingPane* pane = nullptr;
    if(group == ResourceManagerPane::PANE_GROUP){
        pane = ResourceManagerPane::open(dockingManager,true);
    }else if(group == VersionControlPane::PANE_GROUP){
        pane = VersionControlPane::open(dockingManager,true);
    }else if(group == FileTransferPane::PANE_GROUP){
        pane = FileTransferPane::open(dockingManager,true);
    }else if(group == FindReplacePane::PANE_GROUP){
        pane = FindReplacePane::open(dockingManager,true);
    }else if(group == ServerManagePane::PANE_GROUP){
        pane = ServerManagePane::open(dockingManager,true);
    }else if(group == TerminalPane::PANE_GROUP){
        pane = TerminalPane::open(dockingManager,true);
    }else if(group == OutputPane::PANE_GROUP){
        pane = OutputPane::open(dockingManager,true);
    }else if(group == NotificationPane::PANE_GROUP){
        pane = NotificationPane::open(dockingManager,true);
    }
    return pane;
}

PaneLoader::PaneLoader()
{

}

}
