#include "localdirpanel.h"
#include "ui_localdirpanel.h"
#include "local/FileItemModel.h"
#include "components/DirComboBox.h"
#include "components/message_dialog.h"
#include "common.h"
#include "interface/Panel.h"
#include "interface/NewFileDialog.h"
#include "transfer/Task.h"
#include "transfer/TaskPoolModel.h"
#include "mainwindow.h"
#include "common/utils.h"
#include "cvs/commit_model.h"
#include "cvs/diff_file_model.h"
#include "cvs/branch.h"
#include "cvs/git/git_repository.h"
#include "cvs/svn/svn_repository.h"
#include "zip/zip_archive.h"
#include "storage/commit_storage.h"
#include "storage/FavoriteStorage.h"
#include "modules/favorite/AddDialog.h"
#include "modules/favorite/ManageDialog.h"

#include <QToolButton>
#include <QtConcurrent>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QMenu>
#include <QCursor>
#include <QScrollBar>
#include <QItemSelectionModel>
#include <QThread>
#include <QDir>
#include <QDebug>
namespace ady {

LocalDirPanel::LocalDirPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LocalDirPanel)
{
    ui->setupUi(this);
    m_repo = nullptr;

    m_dirComboBox = new DirComboBox(ui->toolBar);
    m_dirComboBox->setModel(new DirModel);
    m_dirComboBox->setEditable(true);
    m_branchComboBox = new QComboBox(ui->cvsToolBar);
    m_model = new FileItemModel(ui->treeView);
    ui->treeView->setEditTriggers(QAbstractItemView::EditKeyPressed);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeView->header()->setSortIndicator(0,Qt::AscendingOrder);
    ui->treeView->setModel(m_model);
    ui->treeView->setColumnWidth(0,180);
    ui->treeView->setColumnWidth(1,80);
    ui->treeView->setColumnWidth(2,125);
    ui->treeView->addMimeType(MM_DOWNLOAD);
    ui->toolBar->addWidget(m_dirComboBox);
    ui->cvsToolBar->insertWidget(ui->actionCVSRefresh,m_branchComboBox);
    m_dirComboBox->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

    m_currentDir = QStandardPaths::standardLocations(QStandardPaths::HomeLocation)[0];
    m_model->readDir(m_currentDir);
    ((DirComboBox*)m_dirComboBox)->setPath(m_currentDir);

    ui->treeView->setDragEnabled(true);



    CommitModel* commitModel = new CommitModel(ui->commitTreeView);
    ui->commitTreeView->setModel(commitModel);
    CommitItemDelegate* commitItemDelegate = new CommitItemDelegate(ui->commitTreeView);
    ui->commitTreeView->setItemDelegate(commitItemDelegate);
    ui->commitTreeView->setColumnWidth(0,60);
    ui->commitTreeView->setColumnWidth(1,120);

    QScrollBar* scrollBar = ui->commitTreeView->verticalScrollBar();
    if(scrollBar!=nullptr){
        connect(scrollBar,SIGNAL(valueChanged(int)),this,SLOT(onMoreCommitLists(int)));
    }


    DiffFileModel* diffFileModel = new DiffFileModel(ui->deltaTreeView);
    ui->deltaTreeView->setModel(diffFileModel);
    ui->deltaTreeView->setColumnWidth(1,75);
    ui->deltaTreeView->setColumnWidth(2,120);
    ui->deltaTreeView->header()->setSectionResizeMode(0,QHeaderView::Stretch);
    //ui->deltaTreeView->header()->setSectionResizeMode(1,QHeaderView::Fixed);
    //ui->deltaTreeView->header()->setSectionResizeMode(2,QHeaderView::Fixed);

    //ui->deltaTreeView->setCol


    connect( ui->treeView,SIGNAL(activated(const QModelIndex &)),this,SLOT(onDirTreeViewActivated(const QModelIndex& )));
    connect( m_dirComboBox,SIGNAL(editTextChanged(const QString &)),this,SLOT(onDirComboBoxChanged(const QString &)));
    connect(ui->treeView->header(),SIGNAL(sortIndicatorChanged(int ,Qt::SortOrder)),m_model,SLOT(sortList(int,Qt::SortOrder)));

    connect(ui->treeView,SIGNAL(dropFinished(const QMimeData*)),this,SLOT(onDropDownload(const QMimeData*)));
    connect(ui->treeView,SIGNAL(customContextMenuRequested(const QPoint&)),this,SLOT(onContextMenu(const QPoint&)));

    connect( ui->commitTreeView,SIGNAL(clicked(const QModelIndex &)),this,SLOT(onCommitItemClicked(const QModelIndex& )));
    connect(ui->commitTreeView->selectionModel(),SIGNAL(currentRowChanged(const QModelIndex &,const QModelIndex &)),this,SLOT(onCommitItemChanged(const QModelIndex &,const QModelIndex &)));
    connect(ui->commitTreeView,SIGNAL(customContextMenuRequested(const QPoint&)),this,SLOT(onCommitContextMenu(const QPoint&)));




    connect(ui->deltaTreeView,SIGNAL(customContextMenuRequested(const QPoint&)),this,SLOT(onDiffFileContextMenu(const QPoint&)));

    connect(ui->actionRefresh,&QAction::triggered,this,&LocalDirPanel::onActRefresh);

    connect(ui->actionCVSRefresh,&QAction::triggered,this,&LocalDirPanel::onActCVSRefresh);
    connect(ui->actionDiff_Workdir,SIGNAL(triggered(bool)),this,SLOT(onActDiffWorkdir(bool)));
    connect(ui->actionDiff_Commits,SIGNAL(triggered(bool)),this,SLOT(onActDiffCommits(bool)));
    connect(ui->actionZip,&QAction::triggered,this,&LocalDirPanel::onActCVSZip);


    connect(ui->actionFavorite,&QAction::triggered,this,&LocalDirPanel::onActAddFavorite);
    //onActAddFavorite
    //ui->actionFavorite->setMenu(menu);

    /*QToolButton* w = (QToolButton*)ui->toolBar->widgetForAction(ui->actionFavorite);
    QMenu *menu = new QMenu(w);
    menu->addAction(ui->actionZip);
    w->setPopupMode(QToolButton::MenuButtonPopup);
    w->setMenu(menu);*/

}

void LocalDirPanel::onContextMenu(const QPoint& pos)
{
    Q_UNUSED(pos);
    QMenu* menu = new QMenu;
    menu->addAction(tr("Upload"),this,SLOT(onActUpload()));
    if(m_sites.size()>1){
        QList<long long> groupids;
        QMenu* subMenu = menu->addMenu(tr("Upload To"));
        QList<SiteRecord>::iterator iter = m_sites.begin();
        while(iter!=m_sites.end()){
            groupids.push_back((*iter).groupid);
            QAction* action = subMenu->addAction((*iter).name,this,SLOT(onActUploadTo()));
            QMap<QString,QVariant> data;
            data.insert("siteid",(*iter).id);
            action->setData(data);
            iter++;
        }

        if(m_groups.size()>0){
            subMenu->addSeparator();
            for(auto g : m_groups){
                QAction* action = subMenu->addAction(g.name,this,SLOT(onActUploadTo()));
                action->setEnabled(groupids.indexOf(g.id)>-1);
                QMap<QString,QVariant> data;
                data.insert("groupid",g.id);
                action->setData(data);
            }
        }

    }
    menu->addSeparator();
    menu->addAction(tr("Open"),this,SLOT(onActOpen()));
    menu->addAction(tr("Rename"),this,SLOT(onActRename()));
    menu->addAction(tr("Delete"),this,SLOT(onActDelete()));
    menu->addSeparator();
    menu->addAction(tr("New Folder"),this,SLOT(onActNewFolder()));
    menu->addAction(tr("New File"),this,SLOT(onActNewFile()));
    menu->addSeparator();
    menu->addAction(tr("Refresh"),this,SLOT(onActRefresh()));
    menu->exec(QCursor::pos());
}

void LocalDirPanel::onDirTreeViewActivated(const QModelIndex & index)
{

    int row = index.row();
    FileItem item = m_model->getItem(row);
    if(item.enabled){
        QFileInfo fi(item.path);
        QString path = fi.absoluteFilePath();
        ((DirComboBox*)m_dirComboBox)->setPath(path);
        this->openDir(path);
        ui->treeView->reset();
    }
}

void LocalDirPanel::onDirComboBoxChanged(const QString & path)
{
    QFileInfo fi(path);
    if(fi.exists() && fi.isDir()){
        ((DirComboBox*)m_dirComboBox)->setPath(fi.absoluteFilePath());
        //m_model->readDir();
        this->openDir(fi.absoluteFilePath());
        ui->treeView->reset();
    }
}

void LocalDirPanel::onDropDownload(const QMimeData* data)
{
    //qDebug()<<"onDropDownload";
    if(data->hasFormat(MM_DOWNLOAD)){
        QString local = m_currentDir;
        QByteArray b = data->data(MM_DOWNLOAD);
        QDataStream out(&b,QIODevice::ReadOnly);
        long long id = 0;
        out >> id;
        //QStringList list;
        qint64 p;
        out >> p;
        QList<FileItem>* lists = (QList<FileItem>*)p;
        QList<Task*> tasks;
        foreach(auto one,*lists){
            //qDebug()<<one.name;
            Task* t = new Task();

            t->cmd = Panel::CMD::DOWNLOAD;
            t->siteid = id;
            t->remote = one.path;
            t->local = local + "/" + one.name;
            t->type = one.type==FileItem::File?0:1;
            t->filesize = one.size;
            if(t->remote.endsWith("/")){
                t->remote = t->remote.left(t->remote.length() - 1);
            }
            tasks.push_back(t);
        }
        //qDebug()<<"id:"<<id;
        delete lists;
        onCommand(tasks);
        /*if(tasks.size()>0){
            TaskPoolModel::getInstance()->appendTasks(tasks);
        }*/
    }
}

void LocalDirPanel::onTestA()
{
    qDebug()<<"test";
}

void LocalDirPanel::onCommand(int cmd,long long id,QList<FileItem> items,QString dstDir,bool keepDir)
{
    if(items.size()>0){
        QList<Task*> tasks;
        QList<FileItem>::iterator iter = items.begin();
        while(iter!=items.end()){
            Task* t = new Task();
            t->cmd = cmd;
            t->siteid = id;
            if(cmd==Panel::CMD::UPLOAD){
                t->local = (*iter).path;
                t->remote = "";
            }else if(cmd==Panel::CMD::DOWNLOAD){
                t->remote = (*iter).path;
                t->local = "";
            }else if(cmd==Panel::CMD::DEL){
                t->remote = (*iter).path;
                t->local = "";
            }else if(cmd==Panel::CMD::CHMOD){
                t->remote = (*iter).path;
                t->local = "";
            }
            tasks.push_back(t);
            iter++;
        }
        TaskPoolModel::getInstance()->appendTasks(tasks);
    }

}

void LocalDirPanel::onCommand(QList<Task*> tasks)
{
    TaskPoolModel::getInstance()->appendTasks(tasks);
    MainWindow* mainWindow = static_cast<MainWindow*>(this->topLevelWidget());
    mainWindow->onTaskExpandAll();
}

void LocalDirPanel::onDownload(long long id,QList<FileItem> lists)
{

}


void LocalDirPanel::onActUpload()
{
    QList<FileItem> items;
    MainWindow* mainWindow = static_cast<MainWindow*>(this->topLevelWidget());
    bool cvsMode = mainWindow->cvsMode();
    if(cvsMode){
        DiffFileModel* model = (DiffFileModel*)ui->deltaTreeView->model();
        QModelIndexList lists =  ui->deltaTreeView->selectionModel()->selectedRows();
        QString projectDir = m_projectDir;
        if(!projectDir.endsWith("/")){
            projectDir += "/";
        }
        for(auto one:lists){
            cvs::DiffFile r = model->getItem(one.row());
            if(r.status()==cvs::DiffFile::Addition || r.status()==cvs::DiffFile::Change){
                FileItem item;
                item.path = projectDir + r.path();
                QFileInfo fi(item.path);
                if(fi.exists()){
                    item.name = fi.fileName();
                    item.size = fi.size();
                    item.type = FileItem::File;
                    item.modifyTime = fi.fileTime(QFileDevice::FileModificationTime).toString("yyyy-MM-dd HH:mm");
                    items.push_back(item);
                }
            }
        }
    }else{
        QModelIndexList lists =  ui->treeView->selectionModel()->selectedRows();
        for(auto one:lists){
            items.push_back(m_model->getItem(one.row()));
        }
    }
    if(items.size()>0){
        QMap<long long,Panel*> panels = mainWindow->panels();
        if(panels.size()>0){
            char cmd = 0;
            QMap<long long,Panel*>::iterator iter = panels.begin();
            while(iter!=panels.end()){
                QList<Task*> tasks = (*iter)->matchFilePath(items,cmd);
                TaskPoolModel::getInstance()->appendTasks(tasks);
                iter++;
            }
        }
        mainWindow->onTaskExpandAll();
    }
}

void LocalDirPanel::onActUploadTo()
{
    QAction* a = static_cast<QAction*>(sender());
    QVariant v = a->data();
    QMap<QString,QVariant> data = v.toMap();
    QList<long long>ids;
    if(data.contains("groupid")){
        long long groupid = data["groupid"].toLongLong();
        for(auto site:m_sites){
            if(site.groupid == groupid){
                ids.push_back(site.id);
            }
        }
    }else if(data.contains("siteid")){
        ids.push_back(data["siteid"].toLongLong());
    }

    if(ids.size()>0){
        MainWindow* mainWindow = static_cast<MainWindow*>(this->topLevelWidget());
        bool cvsModel = mainWindow->cvsMode();
        QList<FileItem> items;
        if(cvsModel){
            //cvs context menu upload
            DiffFileModel* model = (DiffFileModel*)ui->deltaTreeView->model();
            QModelIndexList lists = ui->deltaTreeView->selectionModel()->selectedRows();
            QString projectDir = m_projectDir;
            if(!projectDir.endsWith("/")){
                projectDir += "/";
            }
            for(auto one:lists){
                cvs::DiffFile r = model->getItem(one.row());
                if(r.status()==cvs::DiffFile::Addition || r.status()==cvs::DiffFile::Change){
                    FileItem item;
                    item.path = projectDir + r.path();
                    QFileInfo fi(item.path);
                    if(fi.exists()){
                        item.name = fi.fileName();
                        item.size = fi.size();
                        item.type = FileItem::File;
                        item.modifyTime = fi.fileTime(QFileDevice::FileModificationTime).toString("yyyy-MM-dd HH:mm");
                        items.push_back(item);
                    }
                }
            }
            //qDebug()<<"item size:"<<items.size();
        }else{
            QModelIndexList lists = ui->treeView->selectionModel()->selectedRows();
            for(auto one:lists){
                items.push_back(m_model->getItem(one.row()));
            }
        }
        if(items.size()>0){
            for(auto id:ids){
                char cmd = 0;
                Panel* panel = mainWindow->panel(id);
                if(panel!=nullptr){
                    QList<Task*> tasks = panel->matchFilePath(items,cmd);
                    TaskPoolModel::getInstance()->appendTasks(tasks);
                }
            }
            mainWindow->onTaskExpandAll();
        }

    }
}

void LocalDirPanel::onActOpen()
{
   QModelIndex index = ui->treeView->currentIndex();
   int row = index.row();
   if(row>=0 && row<m_model->rowCount()){
       FileItem item = m_model->getItem(row);
       if(item.path.isEmpty()==false){
            QUrl url = QUrl(item.path);
            QDesktopServices::openUrl(url);
       }
   }
}

void LocalDirPanel::onActRename()
{
    QModelIndex index = ui->treeView->currentIndex();
    int row = index.row();
    if(row>=0 && row<m_model->rowCount()){
        FileItem item = m_model->getItem(index.row());
        ui->treeView->edit(index);
    }
}

void LocalDirPanel::onActDelete()
{
    if(MessageDialog::confirm(this->topLevelWidget(),tr("Are you want to delete selected items?"))==QMessageBox::Yes){
       QItemSelectionModel* model =  ui->treeView->selectionModel();
       if(model!=nullptr){
            QModelIndexList indexes = model->selectedRows();
            if(indexes.size()>0){
                QModelIndexList::iterator iter = indexes.begin();
                while(iter!=indexes.end()){
                    FileItem item = m_model->getItem((*iter).row());
                    Utils::deleteFile(item.path);
                    iter++;
                }
                this->onActRefresh();
            }
       }
    }
}

void LocalDirPanel::onActNewFolder()
{
    NewFileDialog* dialog = new NewFileDialog(NewFileDialog::Folder,this->topLevelWidget());
    dialog->setPath(m_currentDir);
    int ret = dialog->exec();
    if(ret == QDialog::Accepted){
        QString filename = dialog->fileName();
        QFileInfo fi(filename);
        if(fi.absoluteDir().mkdir(fi.fileName())){
            this->onActRefresh();
        }else{
            MessageDialog::error(this->topLevelWidget(),tr("Folder creation failed"));
        }
    }
    delete dialog;
}

void LocalDirPanel::onActNewFile()
{
    NewFileDialog* dialog = new NewFileDialog(NewFileDialog::File,this->topLevelWidget());
    dialog->setPath(m_currentDir);
    int ret = dialog->exec();
    if(ret == QDialog::Accepted){
        QString filename = dialog->fileName();
        QFile file(filename);
        if (file.open(QIODevice::ReadWrite | QIODevice::Text)){
            file.close();
            this->onActRefresh();
        }else{
            MessageDialog::error(this->topLevelWidget(),tr("File creation failed"));
        }
    }
    delete dialog;
}
void LocalDirPanel::onActRefresh()
{
    this->openDir(m_currentDir);
}

void LocalDirPanel::onActSyncDelete()
{
    QAction* a = static_cast<QAction*>(sender());
    QVariant v = a->data();
    QMap<QString,QVariant> data = v.toMap();
    QList<long long>ids;
    if(data.contains("groupid")){
        long long groupid = data["groupid"].toLongLong();
        for(auto site:m_sites){
            if(site.groupid == groupid){
                ids.push_back(site.id);
            }
        }
    }else if(data.contains("siteid")){
        ids.push_back(data["siteid"].toLongLong());
    }else{
        for(auto site:m_sites){
            ids.push_back(site.id);
        }
    }

    QModelIndexList lists = ui->deltaTreeView->selectionModel()->selectedRows();
    if(lists.size()>0){
        DiffFileModel* model = (DiffFileModel*)ui->deltaTreeView->model();
        QString projectDir = this->m_projectDir;
        if(!projectDir.endsWith("/")){
            projectDir += "/";
        }
        QList<FileItem> items;
        for(auto one:lists){
            cvs::DiffFile r = model->getItem(one.row());
            if(r.status()==cvs::DiffFile::Deletion){
                FileItem item;
                QString path = projectDir + r.path();
                QFileInfo fi(path);
                if(fi.exists()==false){
                    item.path = path;
                    item.name = fi.fileName();
                    item.size = fi.size();
                    item.type = FileItem::File;
                    item.modifyTime = fi.fileTime(QFileDevice::FileModificationTime).toString("yyyy-MM-dd HH:mm");
                    items.push_back(item);
                }
            }
        }
        if(items.size()>0){
            MainWindow* mainWindow = static_cast<MainWindow*>(this->topLevelWidget());
            QMap<long long,ady::Panel*> panels = mainWindow->panels();
            QMap<long long,ady::Panel*>::iterator iter = panels.begin();
            while(iter!=panels.end()){
                if(ids.contains(iter.key())){
                    QList<Task*> tasks = (*iter)->matchFilePath(items,0);
                    QList<FileItem> items;
                    for(auto task:tasks){
                        FileItem item;
                        item.path = task->remote;
                        item.type = task->type==0?FileItem::File:FileItem::Dir;
                        QFileInfo fi(task->local);
                        item.name = fi.fileName();
                        items.push_back(item);
                    }
                    qDeleteAll(tasks);
                    tasks.clear();
                    (*iter)->deleteFiles(items);
                }
                iter++;
            }
        }
    }
}

void LocalDirPanel::onSwitchCVS(bool checked)
{
    ui->stackedWidget->setCurrentIndex(checked?1:0);
    if(checked){
        if(m_repo==nullptr){
            QDir d(m_project.path);
#ifndef Q_OS_MAC
            if(d.exists(".svn")){
                m_repo = cvs::Repository::getInstance<cvs::SvnRepository>();
            }else{
                m_repo = cvs::Repository::getInstance<cvs::GitRepository>();
            }

#else
             m_repo = cvs::Repository::getInstance<cvs::GitRepository>();
#endif

            m_repo->init(m_projectDir);
            //ui->action

        }
        ui->actionDiff_Workdir->setChecked(true);
        ui->actionDiff_Commits->setChecked(false);

        this->initBranch();


        {
            CommitModel* model = (CommitModel*)ui->commitTreeView->model();
            if(model->rowCount()==0){
                QList<cvs::Commit> lists = m_repo->commitLists(100);
                this->commitFlags(lists);//fix flags
                model->setList(lists);
            }
        }

        {
            DiffFileModel* model = (DiffFileModel*)ui->deltaTreeView->model();

            QtConcurrent::run([=]()
            {
                model->setList(m_repo->diffFileLists());
            });
        }


    }
}

void LocalDirPanel::onActCVSRefresh()
{
    if(m_repo!=nullptr){
        m_repo->freeRevwalk();
    }

    this->initBranch();
    onActDiffCommits(false);
    {
        ui->commitTreeView->verticalScrollBar()->setValue(0);
        CommitModel* model = (CommitModel*)ui->commitTreeView->model();
        //model->clearCompareRows();
        model->setList(QList<cvs::Commit>());
    }

    //ui->actionDiff_Workdir->setChecked(true);
    //onActDiffWorkdir(true);

    onSwitchCVS(true);
}

void LocalDirPanel::onActCVSClear()
{
    {
        ui->commitTreeView->verticalScrollBar()->setValue(0);
        CommitModel* model = (CommitModel*)ui->commitTreeView->model();
        model->setList(QList<cvs::Commit>());
    }

    {
        ui->deltaTreeView->verticalScrollBar()->setValue(0);
        DiffFileModel* diffModel = (DiffFileModel*)ui->deltaTreeView->model();
        diffModel->clear();
    }

    m_branchComboBox->clear();
}

void LocalDirPanel::onActDiffWorkdir(bool checked)
{
    DiffFileModel* diffModel = (DiffFileModel*)ui->deltaTreeView->model();
    if(checked){
        ui->actionDiff_Commits->setChecked(false);
        onActDiffCommits(false);

        ui->commitTreeView->clearSelection();
        QtConcurrent::run([=]()
        {
            diffModel->setList(m_repo->diffFileLists());
        });
       // QList<cvs::DiffFile> lists = m_repo->diffFileLists();
        //diffModel->setList(lists);
    }else{
        diffModel->clear();
    }
}

void LocalDirPanel::onActDiffCommits(bool checked)
{
    if(checked){
        ui->actionDiff_Workdir->setChecked(false);
        onActDiffWorkdir(false);

        ui->commitTreeView->clearSelection();
        ui->commitTreeView->setSelectionMode(QAbstractItemView::NoSelection);
    }else{
        ui->commitTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
        //reset compare selection row background color
        CommitModel* model = (CommitModel*)ui->commitTreeView->model();
        //model->clearCompareRows();

        {
            DiffFileModel* model = (DiffFileModel*)ui->deltaTreeView->model();
            model->setList(QList<cvs::DiffFile>());
        }

    }
}

void LocalDirPanel::onActCVSZip()
{
    if(MessageDialog::confirm(this->topLevelWidget(),tr("Are you sure compress the diff files to a zip archive?"))==QMessageBox::Yes){
        DiffFileModel* model = (DiffFileModel*)ui->deltaTreeView->model();
        QAction* zipTool = (QAction*)sender();
        bool selected = zipTool->data().toBool();
        QList<cvs::DiffFile> lists;
        if(selected){
            QModelIndexList indexlist = ui->deltaTreeView->selectionModel()->selectedRows();
            for(auto index:indexlist){
                lists.push_back(model->getItem(index.row()));
            }
        }else{
            lists = model->lists();
        }
        if(lists.length()>0){
            ZipArchive zip;
            QString filename = QString("/%1.zip").arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss"));
            //qDebug()<<"zip file:"<<filename;
            if(zip.open(m_projectDir+filename,ZipArchive::Overwrite)){
                for(auto item:lists){
                    if(item.status()!=cvs::DiffFile::Deletion){
                        zip.addFile(m_projectDir+"/"+item.path(),item.path());
                    }
                }
                zip.close();
                MessageDialog::info(topLevelWidget(),tr("Compressed successfully"));
            }else{
                qDebug()<<"zip open failed:"<<zip.errorCode();
            }
        }

    }
}

void LocalDirPanel::onActCommitFlag(bool checked)
{
    QAction* action = (QAction*)sender();
    unsigned int flag = action->data().toInt();
    //int new_flag = checked?flag:~flag;

    QModelIndexList indexlist = ui->commitTreeView->selectionModel()->selectedRows();
    if(indexlist.size()>0 && m_project.id>0l){
        QList<cvs::Commit> lists;
        CommitModel* model = (CommitModel*)ui->commitTreeView->model();
        CommitStorage db(m_project.id);
        int top = -1;
        int bottom = -1;
        for(auto index : indexlist){
            int row = index.row();
            if(top<0){
                top = row;
            }
            if(bottom < row){
                bottom = row;
            }
            cvs::Commit item = model->getItem(row);
            unsigned int flags = item.flag();
            qDebug()<<"flags old:"<<flags;

            if(checked){
                flags |= flag;
            }else{
                flags &= (~flag);
            }
            db.setFlag(item,flags);
            qDebug()<<"flags new:"<<flag;
            qDebug()<<"flags result:"<<flags;
            item.setFlagValue(flags);
            lists.push_back(item);
        }
        qDebug()<<"top:"<<top<<";bottom:"<<bottom;
        model->updateFlags(lists);
    }
}

void LocalDirPanel::onActCommitRemoveFlags()
{
    QModelIndexList indexlist = ui->commitTreeView->selectionModel()->selectedRows();
    if(indexlist.size()>0 && m_project.id>0l){
        QList<cvs::Commit> lists;
        CommitModel* model = (CommitModel*)ui->commitTreeView->model();
        CommitStorage db(m_project.id);
        int top = -1;
        int bottom = -1;
        for(auto index : indexlist){
            int row = index.row();
            if(top<0){
                top = row;
            }
            if(bottom < row){
                bottom = row;
            }
            cvs::Commit item = model->getItem(row);
            db.del(item.oid());
            item.setFlagValue(0);
            lists.push_back(item);
        }
        model->updateFlags(lists);
    }
}

void LocalDirPanel::onMoreCommitLists(int i)
{
    QScrollBar* scrollBar = (QScrollBar*)sender();
    if(i==scrollBar->maximum()){
        CommitModel* model = (CommitModel*)ui->commitTreeView->model();
        QList<cvs::Commit> lists = m_repo->commitLists(100);
        if(lists.size()>0){
            this->commitFlags(lists);//fix flags
            model->appendList(lists);
        }

    }
}

void LocalDirPanel::onCommitItemClicked(const QModelIndex& index)
{


}

void LocalDirPanel::onCommitItemChanged(const QModelIndex& current,const QModelIndex& previous)
{
    //QModelIndex index = ui->commitTreeView->currentIndex();
    ui->actionDiff_Workdir->setChecked(false);
    if(!ui->actionDiff_Commits->isChecked()){
        CommitModel* model = (CommitModel*)ui->commitTreeView->model();
        int row = current.row();
        cvs::Commit item = model->getItem(row);
        //QList<cvs::DiffFile> lists = m_repo->diffFileLists(item.oid());
        DiffFileModel* diffModel = (DiffFileModel*)ui->deltaTreeView->model();
        QtConcurrent::run([=]()
        {
            diffModel->setList( m_repo->diffFileLists(item.oid()));
        });
        //diffModel->setList(lists);
    }

}

void LocalDirPanel::onCommitContextMenu(const QPoint& pos)
{
    Q_UNUSED(pos);
    QModelIndexList indexlist = ui->commitTreeView->selectionModel()->selectedRows();
    if(indexlist.size()>0){
        QMenu* menu = new QMenu;
        CommitModel* model = (CommitModel*)ui->commitTreeView->model();
        bool dev = false;
        bool prod = false;
        bool other = false;
        for(auto index:indexlist){
            cvs::Commit item = model->getItem(index.row());
            dev = (dev || item.flag(cvs::Commit::Dev));
            prod = (prod || item.flag(cvs::Commit::Prod));
            other = (other || item.flag(cvs::Commit::Other));
        }
        QAction* flagDevAction = menu->addAction(tr("Flag Dev"),this,SLOT(onActCommitFlag(bool)));
        QAction* flagProdAction = menu->addAction(tr("Flag Prod"),this,SLOT(onActCommitFlag(bool)));
        QAction* flagOtherAction = menu->addAction(tr("Flag Other"),this,SLOT(onActCommitFlag(bool)));
        flagDevAction->setIcon(QIcon(":/img/Resource/flag_dev.svg"));
        flagProdAction->setIcon(QIcon(":/img/Resource/flag_prod.svg"));
        flagOtherAction->setIcon(QIcon(":/img/Resource/flag_other.svg"));

        flagDevAction->setCheckable(true);
        flagProdAction->setCheckable(true);
        flagOtherAction->setCheckable(true);
        flagDevAction->setChecked(dev);
        flagProdAction->setChecked(prod);
        flagOtherAction->setChecked(other);

        flagDevAction->setData(cvs::Commit::Dev);
        flagProdAction->setData(cvs::Commit::Prod);
        flagOtherAction->setData(cvs::Commit::Other);

        menu->addSeparator();
        menu->addAction(tr("Remove flags"),this,SLOT(onActCommitRemoveFlags()));

        menu->exec(QCursor::pos());
    }





}

void LocalDirPanel::onDiffFileContextMenu(const QPoint& pos)
{
    Q_UNUSED(pos);
    QMenu* menu = new QMenu;
    menu->addAction(tr("Upload"),this,SLOT(onActUpload()));
    /*QAction* action = menu->addAction(tr("Upload All"),this,SLOT(onActUpload()));
    QMap<QString,QVariant> data;
    data.insert("all",1);
    action->setData(data);*/
    if(m_sites.size()>1){
        QList<long long> groupids;
        QMenu* subMenu = menu->addMenu(tr("Upload To"));
        QList<SiteRecord>::iterator iter = m_sites.begin();
        while(iter!=m_sites.end()){
            groupids.push_back((*iter).groupid);
            QAction* action = subMenu->addAction((*iter).name,this,SLOT(onActUploadTo()));
            QMap<QString,QVariant> data;
            data.insert("siteid",(*iter).id);
            action->setData(data);
            iter++;
        }

        if(m_groups.size()>0){
            subMenu->addSeparator();
            for(auto g : m_groups){
                QAction* action = subMenu->addAction(g.name,this,SLOT(onActUploadTo()));
                action->setEnabled(groupids.indexOf(g.id)>-1);
                QMap<QString,QVariant> data;
                data.insert("groupid",g.id);
                action->setData(data);
            }
        }
    }


    menu->addSeparator();
    menu->addAction(tr("Compress selected"),this,SLOT(onActCVSZip()))->setData(true);


    menu->addSeparator();
    menu->addAction(tr("Sync Delete"),this,SLOT(onActSyncDelete()));
    if(m_sites.size()>1){
        QList<long long> groupids;
        QMenu* subMenu = menu->addMenu(tr("Sync Delete To"));
        QList<SiteRecord>::iterator iter = m_sites.begin();
        while(iter!=m_sites.end()){
            groupids.push_back((*iter).groupid);
            QAction* action = subMenu->addAction((*iter).name,this,SLOT(onActSyncDelete()));
            QMap<QString,QVariant> data;
            data.insert("siteid",(*iter).id);
            action->setData(data);
            iter++;
        }

        if(m_groups.size()>0){
            subMenu->addSeparator();
            for(auto g : m_groups){
                QAction* action = subMenu->addAction(g.name,this,SLOT(onActSyncDelete()));
                action->setEnabled(groupids.indexOf(g.id)>-1);
                QMap<QString,QVariant> data;
                data.insert("groupid",g.id);
                action->setData(data);
            }
        }
    }
    menu->exec(QCursor::pos());
}


LocalDirPanel::~LocalDirPanel()
{
    delete ui;
}

void LocalDirPanel::openDir(QString dir)
{
    m_currentDir = dir;
    m_model->readDir(dir);
}

bool LocalDirPanel::openProjectDir(QString dir)
{
    QFileInfo fi(dir);
    if(fi.exists()){
        if(m_repo!=nullptr){
            delete m_repo;
            m_repo = nullptr;
        }
        ((DirComboBox*)m_dirComboBox)->setPath(dir);
        DiffFileModel* model = (DiffFileModel*)ui->deltaTreeView->model();
        model->setProjectDir(dir);
        this->m_projectDir = dir;
        this->openDir(this->m_projectDir);
        this->initFavroite();
        return true;
    }else{
        return false;
    }
}

void LocalDirPanel::commitFlags(QList<cvs::Commit>& lists)
{
    int size = lists.size();
    if(size>0 && m_project.id>0l){
        int a = 0,b = 0;
        a = lists[0].time().toSecsSinceEpoch();
        b = lists[size - 1].time().toSecsSinceEpoch();
        int start_time = qMin(a,b);
        int end_time = qMax(a,b);
        CommitStorage db(m_project.id);
        QList<CommitRecord> records = db.lists(start_time,end_time);
        QList<cvs::Commit>::iterator iter = lists.begin();
        while(iter!=lists.end()){
            int count = records.size();
            for(int i=0;i<count;i++){
                auto record = records.at(i);
                if(record.oid == (*iter).oid()){
                    (*iter).setFlag(record.flags);
                    records.removeAt(i);
                    goto  break_records;
                }
            }
            break_records:
            iter++;
        }
    }
}


void LocalDirPanel::initBranch()
{
    m_branchComboBox->clear();
    /*QList<cvs::Branch*> branches = m_repo->branchLists();
    int i = 0;
    for(auto branch:branches){
        m_branchComboBox->addItem(branch->name());
        if(branch->head()){
            m_branchComboBox->setCurrentIndex(i);
        }
        i++;
    }*/
}

void LocalDirPanel::onActAddFavorite(){
    if(m_project.id>0){
        FavoriteAddDialog* dialog = new FavoriteAddDialog(this,m_project.id);
        connect(dialog,&FavoriteAddDialog::onSaved,this,&LocalDirPanel::onFavoriteUpdated);
        QString path = m_currentDir.replace(m_projectDir,"@rpath");
        dialog->setCurrentPath(path);
        dialog->setModal(true);
        dialog->show();
    }
}

void LocalDirPanel::onFavoriteUpdated()
{
    this->removeFavorite();
    this->initFavroite();
}

void LocalDirPanel::onFavoriteItemSelected()
{
    QAction* a = static_cast<QAction*>(sender());
    QVariant data = a->data();
    QString path = data.toString();
    if(path.isNull() || path.isEmpty()){
        FavoriteManageDialog *dialog = new FavoriteManageDialog(this,m_project.id);
        connect(dialog,&FavoriteManageDialog::onSaved,this,&LocalDirPanel::onFavoriteUpdated);
        dialog->setModal(true);
        dialog->show();
    }else{
        path = path.replace("@rpath",m_projectDir);
        this->onDirComboBoxChanged(path);
    }
}

void LocalDirPanel::initFavroite()
{
    FavoriteStorage db;
    QList<FavoriteRecord>lists = db.list(m_project.id);
    QToolButton* tool = (QToolButton*)ui->toolBar->widgetForAction(ui->actionFavorite);
    if(tool!=nullptr){
        QMenu* menu = tool->menu();
        if(lists.size()>0){
            if(menu==nullptr){
                menu = new QMenu(tool);
                tool->setPopupMode(QToolButton::MenuButtonPopup);
                tool->setMenu(menu);
            }
            Q_FOREACH(FavoriteRecord one,lists){
                QAction* a = menu->addAction(one.name,this,&LocalDirPanel::onFavoriteItemSelected);
                a->setData(one.path);
            }

            menu->addSeparator();
            menu->addAction(tr("Manage"),this,&LocalDirPanel::onFavoriteItemSelected);


        }else{
            if(menu!=nullptr){
                tool->setMenu(nullptr);
                delete menu;
            }
        }


    }
}

void LocalDirPanel::removeFavorite()
{
    QToolButton* tool = (QToolButton*)ui->toolBar->widgetForAction(ui->actionFavorite);
    if(tool!=nullptr){
        tool->setPopupMode(QToolButton::DelayedPopup);
        QMenu* menu = tool->menu();
        if(menu!=nullptr){
            tool->setMenu(nullptr);
            delete menu;
        }
    }
}

}
