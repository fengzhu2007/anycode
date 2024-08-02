#ifndef CODEEDITORREADFILETASK_H
#define CODEEDITORREADFILETASK_H

#include "core/backend_thread.h"
namespace ady{
class CodeEditorReadFileTaskPrivate;
class CodeEditorReadFileTask : public BackendThreadTask
{
public:
    CodeEditorReadFileTask(const QString& path);
    virtual ~CodeEditorReadFileTask();
    QString path();
    virtual bool exec();
private:
    CodeEditorReadFileTaskPrivate* d;
};
}

#endif // CODEEDITORREADFILETASK_H
