#ifndef LOCALDIRPANEL_H
#define LOCALDIRPANEL_H

#include <QWidget>
#include <QComboBox>
#include "local/file_item.h"
#include "storage/site_storage.h"
#include "storage/project_storage.h"
#include "storage/group_storage.h"
#include "cvs/commit.h"
namespace Ui {
class LocalDirPanel;
}


namespace ady {
namespace cvs {
    class Repository;
}

class FileItemModel;
class Task;
class LocalDirPanel : public QWidget
{
    Q_OBJECT

public:

    explicit LocalDirPanel(QWidget *parent = nullptr);
    ~LocalDirPanel();

    void openDir(QString dir);
    bool openProjectDir(QString dir);
    inline void setProject(ProjectRecord& project){m_project = project;}
    inline void setSites(QList<SiteRecord>& sites){m_sites = sites;}
    inline void setGroups(QList<GroupRecord>& groups){m_groups = groups;}



signals:

public slots:
    void onContextMenu(const QPoint& pos);
    void onDirTreeViewActivated(const QModelIndex & index);
    void onDirComboBoxChanged(const QString & path);
    void onDropDownload(const QMimeData* data);
    void onTestA();
    void onCommand(int cmd,long long id,QList<FileItem> items,QString dstDir,bool keepDir);
    void onCommand(QList<Task*> tasks);
    void onDownload(long long id,QList<FileItem> lists);

    /*******action slots*****/
    void onActUpload();
    void onActUploadTo();
    void onActOpen();
    void onActRename();
    void onActDelete();
    void onActNewFolder();
    void onActNewFile();
    void onActRefresh();
    void onActSyncDelete();
    void onActCVSRefresh();
    void onActCVSClear();
    void onActDiffWorkdir(bool checked);
    void onActDiffCommits(bool checked);
    void onActCVSZip();
    void onActCommitFlag(bool checked);
    void onActCommitRemoveFlags();
    void onActAddFavorite();
    void onFavoriteUpdated();
    void onFavoriteItemSelected();

    void onSwitchCVS(bool checked);

    void onMoreCommitLists(int i);
    void onCommitItemClicked(const QModelIndex& index);
    void onCommitItemChanged(const QModelIndex& current,const QModelIndex& previous);

    void onCommitContextMenu(const QPoint& pos);

    void onDiffFileContextMenu(const QPoint& pos);



    void removeFavorite();

private:
    void commitFlags(QList<cvs::Commit>& lists);
    void initBranch();
    void initFavroite();



private:
    Ui::LocalDirPanel *ui;
    FileItemModel* m_model;
    QComboBox* m_branchComboBox;
    QComboBox* m_dirComboBox;
    QString m_projectDir;
    QString m_currentDir;
    QList<SiteRecord> m_sites;
    QList<GroupRecord> m_groups;
    ProjectRecord m_project;
    cvs::Repository* m_repo;

};
}

#endif // LOCALDIRPANEL_H
