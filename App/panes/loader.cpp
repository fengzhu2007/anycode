#include "loader.h"
#include "docking_pane.h"
#include "docking_pane_manager.h"
#include "panes/resource_manager/resource_manager_pane.h"
#include "panes/code_editor/code_editor_pane.h"
#include "panes/version_control/version_control_pane.h"
#include "panes/file_transfer/file_transfer_pane.h"
#include "panes/find_replace/find_replace_pane.h"
#include "panes/server_manage/server_manage_pane.h"

namespace ady{

DockingPane* PaneLoader::init(DockingPaneManager* dockingManager,const QString& group,QJsonObject& data){
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
    }
    return pane;
}

PaneLoader::PaneLoader()
{

}

}