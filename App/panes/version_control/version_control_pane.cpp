#include "version_control_pane.h"
#include "ui_version_control_pane.h"
#include "docking_pane_manager.h"
#include "docking_pane_layout_item_info.h"
#include "panes/resource_manager/resource_manager_model.h"
#include "panes/resource_manager/resource_manager_model_item.h"
#include "cvs/git/git_repository.h"
#include "cvs/svn/svn_repository.h"
#include "cvs/commit_model.h"
#include "cvs/diff_file_model.h"
#include "network/network_response.h"
#include "version_control_query_commit_task.h"
#include "version_control_query_diff_task.h"
#include "version_control_delete_file_task.h"
#include "w_toast.h"
#include "components/message_dialog.h"
#include "zip/zip_archive.h"
#include "storage/commit_storage.h"
#include "core/event_bus/publisher.h"
#include "core/event_bus/type.h"
#include "core/event_bus/event_data.h"
#include "storage/site_storage.h"
#include "version_controller_thread.h"
#include <memory>
#include <QComboBox>
#include <QLabel>
#include <QDir>
#include <QScrollBar>
#include <QItemSelectionModel>
#include <QMenu>
#include <QDesktopServices>
#include <QClipboard>
#include <QDebug>
namespace ady{

VersionControlPane* VersionControlPane::instance = nullptr;



const QString VersionControlPane::PANE_ID = "VersionControl";
const QString VersionControlPane::PANE_GROUP = "VersionControl";



class VersionControlPanePrivate{
public:
    QLabel* branch;
    QLabel* label;
    std::shared_ptr<cvs::Repository> repo;
    long long current_pid=0;
    bool init = false;
    VersionControllerThread* thread;
    QList<SiteRecord> sites;
    QString current_path;
};

VersionControlPane::VersionControlPane(QWidget *parent)
    :DockingPane(parent),wSpin(this),ui(new Ui::VersionControlPane)
{
    d = new VersionControlPanePrivate;
    d->repo = nullptr;
    d->init = false;
    d->thread = nullptr;
    Subscriber::reg();
    this->regMessageIds({Type::M_QUERY_COMMIT,Type::M_QUERY_DIFF});
    QWidget* widget = new QWidget(this);//keep level like createPane(id,group...)
    widget->setObjectName("widget");
    ui->setupUi(widget);
    this->setCenterWidget(widget);
    this->setWindowTitle(tr("Version Control"));
    this->setStyleSheet("QToolBar{border:0px;}"
                        "QTreeView{border:0;background-color:#f5f5f5}"
                        ".ady--ResourceManagerPane>#widget{background-color:#EEEEF2}");
    d->branch = new QLabel(ui->toolBar);
    d->label = new QLabel(ui->toolBar2);
    //d->label->hide();
    //d->label->setVisible(false);
    d->branch->setContentsMargins(0,0,3,0);
    d->label->setContentsMargins(0,0,3,0);
    ui->toolBar->insertWidget(ui->actionRefresh,d->branch);
    ui->toolBar2->insertWidget(ui->actionBackwards,d->label);
    ui->comboBox->setModel(ResourceManagerModel::getInstance());
    ui->commitListView->setModel(new CommitModel(ui->commitListView));
    ui->commitListView->setColumnWidth(CommitModel::DateTime,85);
    ui->commitListView->setColumnWidth(CommitModel::Author,85);
    ui->diffListView->setModel(new DiffFileModel(ui->diffListView));
    ui->diffListView->setTextElideMode(Qt::ElideLeft);
    ui->diffListView->setColumnWidth(DiffFileModel::Name,210);
    ui->diffListView->setColumnWidth(DiffFileModel::ModifyTime,85);

    ui->commitListView->setItemDelegate(new CommitItemDelegate(ui->commitListView));

    ui->commitListView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->diffListView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->commitListView,&QTreeView::customContextMenuRequested, this, &VersionControlPane::onCommitContextMenu);
    connect(ui->diffListView,&QTreeView::customContextMenuRequested, this, &VersionControlPane::onDiffContextMenu);

    connect(ui->comboBox,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&VersionControlPane::onProjectChanged);
    QScrollBar* scrollBar = ui->commitListView->verticalScrollBar();
    if(scrollBar!=nullptr){
        connect(scrollBar,&QScrollBar::valueChanged,this,&VersionControlPane::onQueryCommit);
    }
    connect(ui->commitListView->selectionModel(),&QItemSelectionModel::currentRowChanged,this,&VersionControlPane::onCommitSelected);
    connect(ui->commitListView->selectionModel(),&QItemSelectionModel::selectionChanged,this,&VersionControlPane::onCommitSelectionChanged);


    connect(ui->actionRefresh,&QAction::triggered,this,&VersionControlPane::onActionTriggered);
    connect(ui->actionBackwards,&QAction::triggered,this,&VersionControlPane::onActionTriggered);
    connect(ui->actionCompare_Commits,&QAction::triggered,this,&VersionControlPane::onActionTriggered);
    connect(ui->actionUpload,&QAction::triggered,this,&VersionControlPane::onActionTriggered);
    connect(ui->actionPack_Zip,&QAction::triggered,this,&VersionControlPane::onActionTriggered);

    connect(ui->actionMark_as_Dev,&QAction::triggered,this,&VersionControlPane::onMarkAs);
    connect(ui->actionMark_as_Prod,&QAction::triggered,this,&VersionControlPane::onMarkAs);
    connect(ui->actionMark_as_Other,&QAction::triggered,this,&VersionControlPane::onMarkAs);
    connect(ui->actionClear_Flags,&QAction::triggered,this,&VersionControlPane::onActionTriggered);
    connect(ui->actionClear_All_Flags,&QAction::triggered,this,&VersionControlPane::onActionTriggered);


    connect(ui->actionOpen_File,&QAction::triggered,this,&VersionControlPane::onActionTriggered);
    connect(ui->actionOpen_Folder,&QAction::triggered,this,&VersionControlPane::onActionTriggered);
    connect(ui->actionCopy_Path,&QAction::triggered,this,&VersionControlPane::onActionTriggered);
    connect(ui->actionSynchronous_Deletion,&QAction::triggered,this,&VersionControlPane::onActionTriggered);


    //this->initView();
}


VersionControlPane::~VersionControlPane(){
    Subscriber::unReg();
    instance = nullptr;
    if(d->thread!=nullptr){
        d->thread->requestInterruption();
        d->thread->quit();
    }
    delete d;
    delete ui;
}

void VersionControlPane::initView(){
    //insert project combobox
    if(ui->comboBox->count()>0){
        this->onProjectChanged(0);
    }
}

QString VersionControlPane::id(){
    return PANE_ID;
}

QString VersionControlPane::group(){
    return PANE_GROUP;
}

bool VersionControlPane::onReceive(Event* e){

    return false;
}

void VersionControlPane::queryCommit(bool refresh){
    if(this->isThreadRunning()){
        return ;
    }
    if(refresh){
        //clear model;
        this->clearCommit();
    }
    d->thread = new VersionControllerThread(new VersionControlQueryCommitTask(d->repo));
    connect(d->thread,&VersionControllerThread::finished,this,&VersionControlPane::onFinished);
    d->thread->start();
}

void VersionControlPane::clearCommit(){
    auto model = static_cast<CommitModel*>(ui->commitListView->model());
    model->setDataSource({});
}

void VersionControlPane::queryDiff(const QString& oid1,const QString& oid2){
    if(this->isThreadRunning()){
        return ;
    }
    this->showLoading();
    this->clearDiff();
    //BackendThread::getInstance()->appendTask(new VersionControlQueryDiffTask(d->repo,oid1,oid2));
    d->thread = new VersionControllerThread(new VersionControlQueryDiffTask(d->repo,oid1,oid2));
    connect(d->thread,&VersionControllerThread::finished,this,&VersionControlPane::onFinished);
    d->thread->start();
}

void VersionControlPane::clearDiff(){
    auto model = static_cast<DiffFileModel*>(ui->diffListView->model());
    model->setDataSource({});
}

void VersionControlPane::clearProject(){
    this->clearCommit();
    this->clearDiff();
    d->branch->clear();
    d->label->clear();
    ui->actionRefresh->setEnabled(false);
    ui->actionBackwards->setEnabled(false);
    ui->actionUpload->setEnabled(false);
    ui->actionPack_Zip->setEnabled(false);
    ui->actionCompare_Commits->setEnabled(false);
}

void VersionControlPane::resizeEvent(QResizeEvent *event){
    DockingPane::resizeEvent(event);
    wSpin::resize();
}


void VersionControlPane::onProjectChanged(int i){
    auto model = static_cast<ResourceManagerModel*>(ui->comboBox->model());
    this->clearProject();
    d->current_pid = 0;

    if(model!=nullptr){
        auto index = model->index(i,0);
        auto item = static_cast<ResourceManagerModelItem*>(index.internalPointer());
        if(item!=nullptr){
            d->current_pid = item->pid();
            //init sites
            d->sites = SiteStorage().list(d->current_pid,1);

            ui->actionRefresh->setEnabled(true);
            auto path =  d->current_path = item->path();
            QDir dir(path);
            if(dir.exists(".svn")){
                d->branch->hide();
                d->repo = std::shared_ptr<cvs::Repository>(new cvs::SvnRepository);
            }else if(dir.exists(".git")){
                d->repo = std::shared_ptr<cvs::Repository>(new cvs::GitRepository);
                d->branch->show();
            }else{
                d->branch->hide();
                this->clearCommit();
                wToast::showText(tr("The version control system is not enabled in the current directory."));
                return ;
            }
            this->showLoading();
            d->repo->init(path);
            QString branch = d->repo->headBranch();
            d->branch->setText(tr("Branch:%1").arg(branch.isEmpty()?"--":branch));
            this->queryCommit(true);

            return ;
            //init commit list
        }
    }
}

void VersionControlPane::onUpdateCommit(int rid,void* list){
    this->hideLoading();
    auto commits = static_cast<QList<cvs::Commit>*>(list);
    if(rid==d->repo->rid()){
        if(d->current_pid>0 && commits->size()>0){
            int a = 0,b = 0;
            a = commits->at(0).time().toSecsSinceEpoch();
            b = commits->at(commits->count() - 1).time().toSecsSinceEpoch();
            int start_time = qMin(a,b);
            int end_time = qMax(a,b);
            CommitStorage db(d->current_pid);
            QList<CommitRecord> records = db.lists(start_time,end_time);
            auto iter = commits->begin();
            while(iter!=commits->end()){
                int i = 0;
                for(auto one:records){
                    if((*iter).oid()==one.oid){
                        (*iter).setFlag(one.flags);
                        records.removeAt(i);
                         goto  break_records;
                    }
                    i++;
                }
                break_records:
                iter++;
            }

        }
        auto model = static_cast<CommitModel*>(ui->commitListView->model());
        int count = model->rowCount();
        model->appendList(*commits);
        if(count==0 && d->repo->error().code==0){
            this->queryDiff({},{});//workding dir status
        }
    }
    delete commits;
}

void VersionControlPane::onUpdateDiff(const QString& first,const QString& second,int rid,void* list){
    this->hideLoading();
    auto diffs = static_cast<QList<cvs::DiffFile>*>(list);
    if(rid==d->repo->rid()){
        auto model = static_cast<DiffFileModel*>(ui->diffListView->model());
        model->setDataSource(*diffs);
        if(!first.isEmpty()){
            ui->actionBackwards->setEnabled(true);
        }else{
            ui->actionBackwards->setEnabled(false);
        }
        if(second.isEmpty()){
            if(first.isEmpty()){
                d->label->setText(tr("Working Dir"));
            }else{
                d->label->setText(tr("Commit:%1").arg(first.left(8)));
            }
        }else{
            d->label->setText(tr("Compare:%1 with %2").arg(first.left(8)).arg(second.left(8)));
        }
        if(diffs->size()>0){
            ui->actionUpload->setEnabled(true);
            ui->actionPack_Zip->setEnabled(true);
        }else{
            ui->actionUpload->setEnabled(false);
            ui->actionPack_Zip->setEnabled(false);
        }
    }
    delete diffs;
}

void VersionControlPane::onQueryCommit(int pos){
    QScrollBar* scrollBar = (QScrollBar*)sender();
    if(pos==scrollBar->maximum()){
        this->queryCommit(false);
    }
}

void VersionControlPane::onCommitSelected(const QModelIndex &current, const QModelIndex &previous){
    auto model = static_cast<CommitModel*>(ui->commitListView->model());
    int row = current.row();
    if(row>=0){
        auto item = model->at(row);
        this->queryDiff(item.oid(),{});
    }
}

void VersionControlPane::onCommitSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected){
    auto model = ui->commitListView->selectionModel();
    if(model->selectedRows().size()>1){
        ui->actionCompare_Commits->setEnabled(true);
    }else{
        ui->actionCompare_Commits->setEnabled(false);
    }
}

void VersionControlPane::onActionTriggered(){
    QObject* sender = this->sender();
    if(sender==ui->actionRefresh){
        if(d->repo!=nullptr){
            d->repo->freeRevwalk();
        }
        {
            ui->commitListView->verticalScrollBar()->setValue(0);
            auto model = static_cast<CommitModel*>(ui->commitListView->model());
            model->setDataSource({});
        }
        int i = ui->comboBox->currentIndex();
        this->onProjectChanged(i);
    }else if(sender==ui->actionBackwards){
        ui->commitListView->selectionModel()->clearSelection();
        this->queryDiff({},{});
    }else if(sender==ui->actionCompare_Commits){
        auto indexes = ui->commitListView->selectionModel()->selectedRows();
        auto model = static_cast<CommitModel*>(ui->commitListView->model());
        auto first = model->at(indexes.constFirst().row());
        auto last = model->at(indexes.constLast().row());
        this->queryDiff(first.oid(),last.oid());
    }else if(sender==ui->actionUpload){

        auto model = static_cast<DiffFileModel*>(ui->diffListView->model());
        auto list = ui->diffListView->selectionModel()->selectedRows();
        QStringList files;
        QString path = d->current_path;
        if(!path.endsWith("/")){
            path += "/";
        }
        for(auto one:list){
            int row = one.row();
            auto item = model->at(row);
            files << path +item.path();
        }
        for(auto site:d->sites){
            this->uploadFiles(site.id,files);
        }

    }else if(sender==ui->actionPack_Zip){
        if(MessageDialog::confirm(this->topLevelWidget(),tr("Are you sure compress the diff files to a zip archive?"))==QMessageBox::Yes){
            this->compressZipPackage(false);
        }
    }else if(sender==ui->actionPack_Selection_To_Zip){
        if(MessageDialog::confirm(this->topLevelWidget(),tr("Are you sure compress the diff files to a zip archive?"))==QMessageBox::Yes){
            this->compressZipPackage(true);
        }
    }else if(sender==ui->actionOpen_File){
        QModelIndexList indexlist = ui->diffListView->selectionModel()->selectedRows();
        if(indexlist.size()>0){
            auto model = static_cast<DiffFileModel*>(ui->diffListView->model());
            auto one = indexlist.at(0);
            auto item = model->at(one.row());
            const QString project_dir = d->repo->path();
            QString path = project_dir + "/" + item.path();
            auto data = OpenEditorData{path,0,0};
            Publisher::getInstance()->post(Type::M_OPEN_EDITOR,&path);
        }
    }else if(sender==ui->actionOpen_Folder){
        QModelIndexList indexlist = ui->diffListView->selectionModel()->selectedRows();
        if(indexlist.size()>0){
            auto model = static_cast<DiffFileModel*>(ui->diffListView->model());
            auto one = indexlist.at(0);
            auto item = model->at(one.row());
            const QString project_dir = d->repo->path();
            QString path = project_dir + "/" + item.path();
            QDesktopServices::openUrl(QFileInfo(path).absoluteDir().absolutePath());
        }
    }else if(sender==ui->actionCopy_Path){
        QModelIndexList indexlist = ui->diffListView->selectionModel()->selectedRows();
        if(indexlist.size()>0){
            auto model = static_cast<DiffFileModel*>(ui->diffListView->model());
            auto one = indexlist.at(0);
            auto item = model->at(one.row());
            const QString project_dir = d->repo->path();
            QString path = project_dir + "/" + item.path();
            //qDebug()<<"path"<<path;
            QApplication::clipboard()->setText(path);
        }
    }else if(sender==ui->actionSynchronous_Deletion){

        auto model = static_cast<DiffFileModel*>(ui->diffListView->model());
        auto list = ui->diffListView->selectionModel()->selectedRows();
        QStringList files;
        QString path = d->current_path;
        if(!path.endsWith("/")){
            path += "/";
        }
        for(auto one:list){
            int row = one.row();
            auto item = model->at(row);
            files << path +item.path();
        }

        this->uploadFiles(-1ll,files);

    }else if(sender==ui->actionClear_Flags){
        QModelIndexList indexlist = ui->commitListView->selectionModel()->selectedRows();
        if(indexlist.size()>0 && d->current_pid>0l){
            QList<cvs::Commit> lists;
            CommitModel* model = (CommitModel*)ui->commitListView->model();
            CommitStorage db(d->current_pid);
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
                cvs::Commit item = model->at(row);
                db.del(item.oid());
                item.setFlagValue(0);
                lists.push_back(item);
            }
            model->updateFlags(lists);
        }
    }else if(sender==ui->actionClear_All_Flags){
        if(d->current_pid>0l){
            if(MessageDialog::confirm(this->topLevelWidget(),tr("Are you sure you want to clear all flags?"))==QMessageBox::Yes){
                CommitStorage db(d->current_pid);
                db.delAll();
                CommitModel* model = (CommitModel*)ui->commitListView->model();
                model->clearAllFlags();
            }
        }
    }

}

void VersionControlPane::onError(int rid,int code,const QString& message){
    if(rid==d->repo->rid())
        wToast::showText(message);
}

void VersionControlPane::onCommitContextMenu(const QPoint& pos){
    Q_UNUSED(pos);
    QModelIndexList indexlist = ui->commitListView->selectionModel()->selectedRows();
    QMenu contextMenu(this);
    if(indexlist.size()>0){
        auto model = static_cast<CommitModel*>(ui->commitListView->model());
        bool dev = false;
        bool prod = false;
        bool other = false;
        for(auto index:indexlist){
            cvs::Commit item = model->at(index.row());
            dev = (dev || item.flag(cvs::Commit::Dev));
            prod = (prod || item.flag(cvs::Commit::Prod));
            other = (other || item.flag(cvs::Commit::Other));
        }



        contextMenu.addAction(ui->actionMark_as_Dev);
        contextMenu.addAction(ui->actionMark_as_Prod);
        contextMenu.addAction(ui->actionMark_as_Other);
        contextMenu.addSeparator();
        contextMenu.addAction(ui->actionClear_Flags);
        contextMenu.addAction(ui->actionClear_All_Flags);


        ui->actionMark_as_Dev->setCheckable(true);
        ui->actionMark_as_Prod->setCheckable(true);
        ui->actionMark_as_Other->setCheckable(true);
        ui->actionMark_as_Dev->setChecked(dev);
        ui->actionMark_as_Prod->setChecked(prod);
        ui->actionMark_as_Other->setChecked(other);


        contextMenu.exec(QCursor::pos());
    }
}

void VersionControlPane::onDiffContextMenu(const QPoint& pos){
    Q_UNUSED(pos);
    QMenu contextMenu(this);
    QModelIndexList indexlist = ui->commitListView->selectionModel()->selectedRows();
    contextMenu.addAction(ui->actionOpen_File);
    contextMenu.addAction(ui->actionOpen_Folder);
    contextMenu.addAction(ui->actionCopy_Path);
    contextMenu.addSeparator();
    contextMenu.addAction(ui->actionUpload);
    //contextMenu.addAction(tr)
    //QMenu* uploadToMenu = contextMenu.addMenu(tr("Upload To"));
    QMenu* uploadToMenu = this->attchUploadMenu(0,&contextMenu);
    if(uploadToMenu!=nullptr){
        contextMenu.addMenu(uploadToMenu);
    }
    contextMenu.addSeparator();
    contextMenu.addAction(ui->actionPack_Zip);
    contextMenu.addAction(ui->actionPack_Selection_To_Zip);
    contextMenu.addAction(ui->actionSynchronous_Deletion);
    //QMenu* deleteMenu = contextMenu.addMenu(tr("Synchronous Deletion To"));
    auto deleteMenu = this->attchUploadMenu(1,&contextMenu);
    if(deleteMenu!=nullptr){
        contextMenu.addMenu(deleteMenu);
    }
    contextMenu.exec(QCursor::pos());
}


void VersionControlPane::onMarkAs(bool checked){
    QObject *sender = this->sender();
    int flag = 0;
    if(sender==ui->actionMark_as_Dev){
        flag = cvs::Commit::Dev;
    }else if(sender==ui->actionMark_as_Prod){
        flag = cvs::Commit::Prod;
    }else if(sender==ui->actionMark_as_Other){
        flag = cvs::Commit::Other;
    }else{
        return ;
    }
    QModelIndexList indexlist = ui->commitListView->selectionModel()->selectedRows();
    if(indexlist.size()>0 && d->current_pid>0l){
        QList<cvs::Commit> lists;
        CommitModel* model = (CommitModel*)ui->commitListView->model();
        CommitStorage db(d->current_pid);
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
            if(checked){
                flags |= flag;
            }else{
                flags &= (~flag);
            }
            db.setFlag(item,flags);
            item.setFlagValue(flags);
            lists.push_back(item);
        }
        model->updateFlags(lists);
    }
}

void VersionControlPane::onFinished(){
    //this->hideLoading();
    //d->thread->wait();
    d->thread->deleteLater();
    d->thread = nullptr;
}

void VersionControlPane::onUploadToSite(){
    auto action = static_cast<QAction*>(this->sender());
    long long siteid = action->data().toLongLong();
    auto model = static_cast<DiffFileModel*>(ui->diffListView->model());
    auto list = ui->diffListView->selectionModel()->selectedRows();
    QStringList files;
    QString path = d->current_path;
    if(!path.endsWith("/")){
        path += "/";
    }
    for(auto one:list){
        int row = one.row();
        auto item = model->at(row);
        files << path +item.path();
    }
    this->uploadFiles(siteid,files);
}
void VersionControlPane::onSynchronousToSite(){
    auto action = static_cast<QAction*>(this->sender());
    long long siteid = action->data().toLongLong();
    auto model = static_cast<DiffFileModel*>(ui->diffListView->model());
    int total = model->rowCount();
    QStringList files;
    QString path = d->current_path;
    if(!path.endsWith("/")){
        path += "/";
    }
    for(int i=0;i<total;i++){
        auto item = model->at(i);
        if(item.status()==cvs::DiffFile::Deletion){
            const QString file = path +item.path();
            if(!QFileInfo::exists(file)){
                files<<file;
            }
        }
    }
    this->deleteFiles(siteid,files);
}
void VersionControlPane::onUploadToGroup(){

}
void VersionControlPane::onSynchronousToGroup(){

}

void VersionControlPane::onOutput(NetworkResponse* response){
    if(response!=nullptr){
        bool status = response->status();
        QJsonObject json = {
            {"level",status?Type::Ok:Type::Error},
            {"source",this->windowTitle()},
            {"content",response->command + "\n" + (status?response->header:response->errorInfo())}
        };
        Publisher::getInstance()->post(Type::M_OUTPUT,json);
        delete response;
    }
}

bool VersionControlPane::isThreadRunning(){
    return d->thread!=nullptr;
}

void VersionControlPane::compressZipPackage(bool selected){
    auto model = static_cast<DiffFileModel*>(ui->diffListView->model());
    QList<cvs::DiffFile> lists;
    if(selected){
        QModelIndexList indexlist = ui->diffListView->selectionModel()->selectedRows();
        for(auto index:indexlist){
            lists.push_back(model->getItem(index.row()));
        }
    }else{
        lists = model->lists();
    }
    if(lists.length()>0){
        ZipArchive zip;
        const QString path = d->repo->path();
        QString filename = QString::fromUtf8("/%1.zip").arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss"));
        if(zip.open(path+filename,ZipArchive::Overwrite)){
            for(auto item:lists){
                if(item.status()!=cvs::DiffFile::Deletion){
                    zip.addFile(path+"/"+item.path(),item.path());
                }
            }
            zip.close();
            wToast::showText(tr("Compressed successfully"));
        }else{
            qDebug()<<"zip open failed:"<<zip.errorCode();
        }
    }
}

void VersionControlPane::uploadFiles(long long siteid,const QStringList& files){
    if(files.size()>0){
        UploadData data{d->current_pid,siteid,true,files.join("|"),{}};//not set dest ,should match remote
        Publisher::getInstance()->post(Type::M_UPLOAD,&data);
    }
}

void VersionControlPane::deleteFiles(long long siteid,const QStringList& files){
    if(this->isThreadRunning()){
        return ;
    }
    if(siteid==-1ll){
        QList<VersionControlDeleteFileTask*> tasks;
        for(auto site:d->sites){
            tasks<<new VersionControlDeleteFileTask(site.id,files);
        }
        if(tasks.size()>0){
            this->showLoading();
            d->thread = new VersionControllerThread(new VersionControlDeleteFileTask(siteid,files));
            connect(d->thread,&VersionControllerThread::finished,this,&VersionControlPane::onFinished);
            d->thread->start();
        }
        return ;
    }
    this->showLoading();
    d->thread = new VersionControllerThread(new VersionControlDeleteFileTask(siteid,files));
    connect(d->thread,&VersionControllerThread::finished,this,&VersionControlPane::onFinished);
    d->thread->start();
}

QMenu* VersionControlPane::attchUploadMenu(int type,QMenu* parent){
    QMenu* uploadM = nullptr;
    for(auto one:d->sites){
        if(uploadM==nullptr){
            uploadM = new QMenu(parent);
            uploadM->setTitle(type==0?tr("Upload To"):tr("Synchronous Deletion To"));
        }
        if(type==0){
            auto action = uploadM->addAction(QIcon(":/Resource/icons/RemoteServer_16x.svg"),one.name,this,&VersionControlPane::onUploadToSite);
            action->setData(one.id);//site id
        }else{
            auto action = uploadM->addAction(QIcon(":/Resource/icons/RemoteServer_16x.svg"),one.name,this,&VersionControlPane::onSynchronousToSite);
            action->setData(one.id);//site id
        }
    }
    return uploadM;
}

VersionControlPane* VersionControlPane::open(DockingPaneManager* dockingManager,bool active){
    if(instance==nullptr){
        instance = new VersionControlPane(dockingManager->widget());
        DockingPaneLayoutItemInfo* item = dockingManager->createPane(instance,DockingPaneManager::Left,active);
        item->setManualSize(300);
    }
    return instance;
}

VersionControlPane* VersionControlPane::make(DockingPaneManager* dockingManager,const QJsonObject& data){
    if(instance==nullptr){
        instance = new VersionControlPane(dockingManager->widget());
        return instance;
    }else{
        return nullptr;
    }
}


}
