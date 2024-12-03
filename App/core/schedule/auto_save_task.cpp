#include "auto_save_task.h"
#include "panes/code_editor/code_editor_manager.h"
#include "core/event_bus/type.h"
#include "core/event_bus/publisher.h"

namespace ady{
AutoSaveTask::AutoSaveTask(int msec):ScheduleTask(msec) {

}

void AutoSaveTask::execute(){
    auto instance = CodeEditorManager::getInstance();
    instance->autoSave();

    Publisher::getInstance()->post(Type::M_READY);
    qDebug()<<"auto save";
}

void AutoSaveTask::doing(){
    QString message = QObject::tr("Auto Save...");
    Publisher::getInstance()->post(Type::M_MESSAGE,&message);
}

}
