#include "find_replace_pane.h"
#include "ui_find_replace_pane.h"
#include "docking_pane_manager.h"
#include "docking_pane_layout_item_info.h"
#include "core/event_bus/event.h"
#include "core/event_bus/event_data.h"
#include "core/event_bus/publisher.h"
#include "core/event_bus/type.h"
#include "find_replace_progress.h"
#include "search_result_model.h"
#include "replace_all_progress_dialog.h"

#include "panes/code_editor/code_editor_manager.h"
#include "panes/code_editor/code_editor_pane.h"
#include "panes/code_editor/code_editor_view.h"
#include "w_toast.h"

//#include "panes/code_editor/code_editor_manager.h"
//#include <QToolButton>



#include <QDebug>

namespace ady{

FindReplacePane* FindReplacePane::instance = nullptr;

const QString FindReplacePane::PANE_ID = "FindAndReplace";
const QString FindReplacePane::PANE_GROUP = "FindAndReplace";

class FindReplacePanePrivate{
public:
    QList<FindReplaceProgress*> list;
};

FindReplacePane::FindReplacePane(QWidget *parent) :
    DockingPane(parent),ui(new Ui::FindReplacePane)
{
    d = new FindReplacePanePrivate;
    Subscriber::reg();
    this->regMessageIds({Type::M_FIND});
    QWidget* widget = new QWidget(this);//keep level like createPane(id,group...)
    widget->setObjectName("widget");
    ui->setupUi(widget);
    this->setCenterWidget(widget);
    this->setWindowTitle(tr("Search Results"));
    this->setStyleSheet("QToolBar{border:0px;}");

    auto model = new SearchResultModel(ui->treeView);
    auto delegate = new SearchResultItemDelegate(1,ui->treeView);
    ui->treeView->setModel(model);
    ui->treeView->setItemDelegate(delegate);

    connect(ui->treeView,&QTreeView::activated,this,&FindReplacePane::onSearchItemActivated);
    connect(ui->actionClear,&QAction::triggered,this,&FindReplacePane::onActionTriggered);
    connect(ui->actionStop,&QAction::triggered,this,&FindReplacePane::stop);
    connect(ui->actionSearch,&QAction::triggered,this,&FindReplacePane::onActionTriggered);
    connect(ui->actionExpand_All,&QAction::triggered,this,&FindReplacePane::onActionTriggered);
    connect(ui->actionCollapse_All,&QAction::triggered,this,&FindReplacePane::onActionTriggered);

    this->initView();
}

FindReplacePane::~FindReplacePane()
{
    Subscriber::unReg();
    delete ui;
    delete d;
    instance = nullptr;
}

void FindReplacePane::initView(){
    ui->label->hide();
    ui->actionStop->setEnabled(false);
}

QString FindReplacePane::id(){
    return PANE_ID;
}

QString FindReplacePane::group(){
    return PANE_GROUP;
}

bool FindReplacePane::onReceive(Event* e){

    return false;
}


void FindReplacePane::runSearch(const QString& text,const QString& scope,int flags,const QString& filter,const QString& exclusion){
    this->stop();
    if(d->list.size()==0){

        //qDebug()<<"scope:"<<scope;
        FindReplaceProgress* progress = nullptr;
        if(scope==":"){
            //current
            auto instance = CodeEditorManager::getInstance();
            if(instance==nullptr){
                return ;
            }
            auto pane = instance->current();
            if(pane==nullptr){
                return ;
            }
            auto doc = pane->editor()->document();
            QString path = pane->path();
            if(path.isEmpty()){
                path = pane->windowTitle();
            }
            progress = new FindReplaceProgress(FindReplaceProgress::SearcnOnly,text,{},flags,path,doc,filter,exclusion);
        }else if(scope=="::"){
            //opened
            auto instance = CodeEditorManager::getInstance();
            auto data = instance->docData();
            if(data.size()==0){
                return ;
            }
             progress = new FindReplaceProgress(FindReplaceProgress::SearcnOnly,text,{},flags,data,filter,exclusion);
        }else{
            //file or folder
             progress = new FindReplaceProgress(FindReplaceProgress::SearcnOnly,text,{},flags,scope,filter,exclusion);
        }

        ui->actionStop->setEnabled(true);
        ui->widget->start();

        d->list << progress;
        connect(progress,&QThread::finished,progress,&QThread::deleteLater);
        connect(progress,&QThread::finished,this,&FindReplacePane::onSearchEnd);
        connect(progress,&FindReplaceProgress::searchResult,this,&FindReplacePane::onSearchResult);
        progress->start();
    }
}

void FindReplacePane::runReplace(const QString& before,const QString& after,const QString& scope,int flags, const QString& filter,const QString& exclusion){
    this->stop();
    if(d->list.size()==0){
        //qDebug()<<"scope"<<scope;
        FindReplaceProgress* progress = nullptr;
        if(scope==":"){
            //current
            auto instance = CodeEditorManager::getInstance();
            if(instance==nullptr){
                return ;
            }
            auto pane = instance->current();
            if(pane==nullptr){
                return ;
            }
            auto doc = pane->editor()->document();
            QString path = pane->path();
            if(path.isEmpty()){
                path = pane->windowTitle();
            }
            progress = new FindReplaceProgress(FindReplaceProgress::SearchAndReplace,before,after,flags,path,doc,filter,exclusion);
        }else if(scope=="::"){
            //opened
            auto instance = CodeEditorManager::getInstance();
            auto data = instance->docData();
            if(data.size()==0){
                return ;
            }
            progress = new FindReplaceProgress(FindReplaceProgress::SearchAndReplace,before,after,flags,data,filter,exclusion);
        }else{
            //file or folder
            progress = new FindReplaceProgress(FindReplaceProgress::SearchAndReplace,before,after,flags,scope,filter,exclusion);
        }

        ui->actionStop->setEnabled(true);
        ui->widget->start();
        d->list << progress;
        connect(progress,&QThread::finished,progress,&QThread::deleteLater);
        connect(progress,&QThread::finished,this,&FindReplacePane::onSearchEnd);
        connect(progress,&FindReplaceProgress::searchResult,this,&FindReplacePane::onSearchResult);
        connect(progress,&FindReplaceProgress::replaceOpendFiles,this,&FindReplacePane::onReplaceOpendFiles);
        progress->start();

        auto dialog = ReplaceAllProgressDialog::open(this);
        connect(dialog,&QDialog::finished,this,&FindReplacePane::onReplaceAllFinished);

    }
}

void FindReplacePane::stop(){
    for(auto one:d->list){
        one->requestInterruption();
        one->quit();
        one->wait();
    }
    d->list.clear();
}

void FindReplacePane::onSearchEnd(){
    auto sender = static_cast<FindReplaceProgress*>(this->sender());
    d->list.removeOne(sender);
    ui->actionStop->setEnabled(false);
    ui->widget->stop();
}

void FindReplacePane::onSearchResult(QList<SearchResultItem>* list,int matchFileCount,int searchFileCount){
    auto model = static_cast<SearchResultModel*>(ui->treeView->model());
    if(!ui->actionPreserve_Results->isChecked()){
        model->clear();
    }
    ui->label->setText(tr("Matched:%1  Matched Files:%2  Search Files:%3").arg(list->size()).arg(matchFileCount).arg(searchFileCount));
    model->appendItems(*list);
    list->clear();
    delete list;
    ui->treeView->expandAll();
    ui->label->show();
}

void FindReplacePane::onSearchItemActivated(const QModelIndex& index){
    auto item = static_cast<SearchResultModelItem*>(index.internalPointer());
    if(item->type()==SearchResultModelItem::Line){
        auto data = OpenEditorData{item->filePath(),item->line(),item->matchStart(),true};
        Publisher::getInstance()->post(Type::M_OPEN_EDITOR,&data);
    }
}

void FindReplacePane::onActionTriggered(){
    auto sender = static_cast<QAction*>(this->sender());
    if(sender==ui->actionStop){
        this->stop();
    }else if(sender==ui->actionSearch){
        auto data = OpenFindData{0,{},{}};
        Publisher::getInstance()->post(Type::M_OPEN_FIND,&data);
    }else if(sender==ui->actionClear){
        auto model = static_cast<SearchResultModel*>(ui->treeView->model());
        model->clear();
        ui->label->hide();
    }else if(sender==ui->actionExpand_All){
        ui->treeView->expandAll();
    }else if(sender==ui->actionCollapse_All){
        ui->treeView->collapseAll();
    }
}

void FindReplacePane::onReplaceOpendFiles(const QStringList& list,const QString& before,const QString& after,int flags,int replaceCount,int replaceFiles){
    auto instance = CodeEditorManager::getInstance();
    if(instance!=nullptr){
        for(auto path:list){
            auto pane = instance->get(path);
            if(pane!=nullptr){
                auto editor = pane->editor();
                replaceCount += editor->replaceAll(before,after,flags);
                replaceFiles += 1;
            }
        }
    }
    auto dialog = ReplaceAllProgressDialog::getInstance();
    if(dialog!=nullptr){
        dialog->hide();
        disconnect(dialog,&QDialog::finished,this,&FindReplacePane::onReplaceAllFinished);
    }
    wToast::showText(tr("Replaced:%1 Replaced Files:%2").arg(replaceCount).arg(replaceFiles));
}

void FindReplacePane::onReplaceAllFinished(int result){
    this->stop();
}

FindReplacePane* FindReplacePane::open(DockingPaneManager* dockingManager,bool active){
    if(instance==nullptr){
        instance = new FindReplacePane(dockingManager->widget());
        DockingPaneLayoutItemInfo* item = dockingManager->createPane(instance,DockingPaneManager::Bottom,active);
        item->setManualSize(220);
    }
    return instance;
}

FindReplacePane* FindReplacePane::make(DockingPaneManager* dockingManager,const QJsonObject& data){
    if(instance==nullptr){
        instance = new FindReplacePane(dockingManager->widget());
        return instance;
    }
    return nullptr;
}

void FindReplacePane::resizeEvent(QResizeEvent *event){
    DockingPane::resizeEvent(event);
}

}
