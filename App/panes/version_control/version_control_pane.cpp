#include "version_control_pane.h"
#include "ui_version_control_pane.h"
#include "docking_pane_manager.h"
#include "docking_pane_layout_item_info.h"
#include "panes/resource_manager/resource_manager_model.h"
#include "panes/resource_manager/resource_manager_model_item.h"
#include "core/backend_thread.h"
#include "cvs/git/git_repository.h"
#include "cvs/svn/svn_repository.h"
#include "cvs/commit_model.h"
#include "cvs/diff_file_model.h"
#include "version_control_query_commit_task.h"
#include "version_control_query_diff_task.h"
#include "w_toast.h"

#include <QComboBox>
#include <QLabel>
#include <QDir>
#include <QScrollBar>
#include <QDebug>
namespace ady{

VersionControlPane* VersionControlPane::instance = nullptr;

const QString VersionControlPane::M_UPLOAD = "Upload";
const QString VersionControlPane::M_QUERY_COMMIT = "QueryCommit";
const QString VersionControlPane::M_QUERY_DIFF = "QueryDiff";


const QString VersionControlPane::PANE_ID = "VersionControl";
const QString VersionControlPane::PANE_GROUP = "VersionControl";



class VersionControlPanePrivate{
public:
    QComboBox* branch;
    QLabel* label;
    cvs::Repository* repo;
    //QThread workerThread;
    //CommitModel* commit_model;
    //DiffFileModel* diff_model;
};

VersionControlPane::VersionControlPane(QWidget *parent)
    :DockingPane(parent),ui(new Ui::VersionControlPane)
{
    d = new VersionControlPanePrivate;
    d->repo = nullptr;
    Subscriber::reg();
    this->regMessageIds({M_QUERY_COMMIT,M_QUERY_DIFF});
    QWidget* widget = new QWidget(this);//keep level like createPane(id,group...)
    ui->setupUi(widget);
    this->setCenterWidget(widget);
    this->setWindowTitle(tr("Version Control"));
    this->setStyleSheet("QToolBar{border:0px;}"
                        "QTreeView{border:0;background-color:#f5f5f5}"
                        ".ady--VersionControlPane>QWidget{background-color:#EEEEF2}");
    d->branch = new QComboBox(widget);
    d->label = new QLabel(widget);
    ui->toolBar->insertWidget(ui->actionRefresh,d->branch);
    ui->toolBar2->insertWidget(ui->actionBackwards,d->label);
    ui->comboBox->setModel(ResourceManagerModel::getInstance());
    ui->commitListView->setModel(new CommitModel(ui->commitListView));
    ui->diffListView->setModel(new DiffFileModel(ui->diffListView));

    connect(ui->comboBox,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&VersionControlPane::onProjectChanged);
    QScrollBar* scrollBar = ui->commitListView->verticalScrollBar();
    if(scrollBar!=nullptr){
        connect(scrollBar,&QScrollBar::valueChanged,this,&VersionControlPane::onQueryCommit);
    }

    connect(ui->commitListView->selectionModel(),&QItemSelectionModel::currentRowChanged,this,&VersionControlPane::onCommitSelected);


    this->initView();
}


VersionControlPane::~VersionControlPane(){
    instance = nullptr;
    if(d->repo!=nullptr)
        d->repo->unUsed();
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
    if(refresh){
        //clear model;
        this->clearCommit();
    }
    BackendThread::getInstance()->appendTask(new VersionControlQueryCommitTask(d->repo));
}

void VersionControlPane::clearCommit(){
    auto model = static_cast<CommitModel*>(ui->commitListView->model());
    model->setDataSource({});
}

void VersionControlPane::queryDiff(const QString& oid1,const QString& oid2){
    this->clearDiff();
    BackendThread::getInstance()->appendTask(new VersionControlQueryDiffTask(d->repo,oid1,oid2));
}

void VersionControlPane::clearDiff(){
    auto model = static_cast<DiffFileModel*>(ui->diffListView->model());
    model->setDataSource({});
}


void VersionControlPane::onProjectChanged(int i){
    auto model = static_cast<ResourceManagerModel*>(ui->comboBox->model());
    if(model!=nullptr){
        auto index = model->index(i,0);
        auto item = static_cast<ResourceManagerModelItem*>(index.internalPointer());
        if(item!=nullptr){
            auto path = item->path();
            QDir dir(path);
            if(d->repo!=nullptr){
                bool ret = d->repo->unUsed();
                if(ret){
                    d->repo = nullptr;
                }
            }
            if(dir.exists(".svn")){
                d->branch->hide();
                d->repo = cvs::Repository::getInstance<cvs::SvnRepository>();
            }else if(dir.exists(".git")){
                d->repo = cvs::Repository::getInstance<cvs::GitRepository>();
                d->branch->show();
            }else{
                d->branch->hide();
                this->clearCommit();
                wToast::showText(tr("The version control system is not enabled in the current directory."));
                return ;
            }
            d->repo->init(path);
            this->queryCommit(true);
            return ;
            //init commit list
        }
    }
    this->clearCommit();
}

void VersionControlPane::onUpdateCommit(void* repo,void* list){
    auto commits = static_cast<QList<cvs::Commit>*>(list);
    if(repo==d->repo){
        auto model = static_cast<CommitModel*>(ui->commitListView->model());
        model->appendList(*commits);
    }
    static_cast<cvs::Repository*>(repo)->unUsed();
    delete commits;
}

void VersionControlPane::onQueryCommit(int pos){
    QScrollBar* scrollBar = (QScrollBar*)sender();
    if(pos==scrollBar->maximum()){
        this->queryCommit(false);
    }
}

void VersionControlPane::onCommitSelected(const QModelIndex &current, const QModelIndex &previous){
    auto model = static_cast<CommitModel*>(ui->diffListView->model());
    //QModelIndex index = ui->diffListView->selectionModel()->currentIndex();
    //qDebug()<<"current:"<<current.row();
    int row = current.row();
    if(row>=0){
        auto item = model->at(row);
        this->queryDiff(item.oid(),{});
    }
}


VersionControlPane* VersionControlPane::open(DockingPaneManager* dockingManager,bool active){
    if(instance==nullptr){
        instance = new VersionControlPane(dockingManager->widget());
        DockingPaneLayoutItemInfo* item = dockingManager->createPane(instance,DockingPaneManager::Left,active);
        item->setStretch(300);
    }
    return instance;
}


}
