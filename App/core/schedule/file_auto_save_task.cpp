#include "file_auto_save_task.h"
#include "panes/code_editor/code_editor_manager.h"
#include "core/event_bus/type.h"
#include "core/event_bus/publisher.h"

namespace ady{
FileAutoSaveTask::FileAutoSaveTask(int msec):ScheduleTask(msec) {

}

void FileAutoSaveTask::execute(){
    auto instance = CodeEditorManager::getInstance();
    instance->autoSave();

    Publisher::getInstance()->post(Type::M_READY);
    //qDebug()<<"auto save";
}

void FileAutoSaveTask::doing(){
    QString message = QObject::tr("Auto Saving...");
    Publisher::getInstance()->post(Type::M_MESSAGE,&message);
}

}
