#include "code_editor_read_file_task.h"



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

}
