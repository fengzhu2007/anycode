#include "code_editor_read_file_task.h"
#include "code_editor_manager.h"


namespace ady{
class CodeEditorReadFileTaskPrivate{
public:
    QString path;
};

CodeEditorReadFileTask::CodeEditorReadFileTask(const QString& path)
    :BackendThreadTask(BackendThreadTask::ReadFile)
{
    d = new CodeEditorReadFileTaskPrivate;
    d->path = path;
}


CodeEditorReadFileTask::~CodeEditorReadFileTask(){
    delete d;
}
QString CodeEditorReadFileTask::path(){

    return d->path;
}

bool CodeEditorReadFileTask::exec(){
    auto instance = CodeEditorManager::getInstance();
    if(instance!=nullptr){
        instance->readFileLines(d->path);
        return true;
    }else{
        return false;
    }
}
}
