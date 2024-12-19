#ifndef BACKUP_RESTORE_H
#define BACKUP_RESTORE_H
#include <QList>
#include "storage/project_storage.h"
#include "storage/site_storage.h"
namespace ady{
class BackupRestorePrivate;
class ProjectRecord;
class SiteRecord;
class BackupRestore
{
public:
    enum Error{
        None=0,
        Invalid_Directory,
        Invalid_Filename,
        Write_Failed,
        Invalid_Json_File,
        Json_Exception,
    };
    BackupRestore();
    ~BackupRestore();
    void appendProject(long long id,const QList<long long>& sites);
    void appendSite(long long id);
    Error save(const QString& directory,const QString& filename);
    Error saveAll(const QString& directory,const QString& filename);
    Error readFile(const QString& path);
    void restore(long long id,const QList<long long>& sites,bool merge=false);

    static QString encode(const QString& str);
    static QString decode(const QString& str);

    static QString defaultFilename();

private:
    ProjectRecord findProject(long long id);
    SiteRecord findSite(long long id);

private:
    BackupRestorePrivate* d;

};
}
#endif // BACKUP_RESTORE_H
