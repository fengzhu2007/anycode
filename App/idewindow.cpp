#include "idewindow.h"
#include "ui_idewindow.h"
#include "docking_pane_manager.h"
#include "docking_workbench.h"
#include "docking_pane.h"
#include "docking_pane_client.h"
#include "docking_pane_layout.h"
#include "docking_pane_layout_item_info.h"
#include "project/open_project_window.h"
#include "project/new_project_window.h"
#include "panes/resource_manager/resource_manager_pane.h"
#include "panes/resource_manager/resource_manager_model.h"
#include "panes/code_editor/code_editor_pane.h"
#include "panes/code_editor/code_editor_manager.h"
#include "panes/version_control/version_control_pane.h"
#include "panes/server_manage/server_manage_pane.h"
#include "panes/file_transfer/file_transfer_pane.h"
#include "panes/find_replace/find_replace_dialog.h"
#include "panes/find_replace/find_replace_pane.h"
#include "panes/file_transfer/file_transfer_model.h"
#include "panes/loader.h"
#include "core/event_bus/event.h"
#include "core/event_bus/publisher.h"
#include "core/event_bus/type.h"
#include "core/event_bus/event_data.h"
#include "core/backend_thread.h"
#include "core/ide_settings.h"
#include "storage/project_storage.h"
#include "network/network_manager.h"
#include "w_toast.h"

#include "languages/html/htmlscanner.h"

#include <QLabel>
#include <QCloseEvent>
#include <QFileDialog>
#include <QTextCodec>
#include <QDebug>

namespace ady{

class IDEWindowPrivate{
public:
    bool init=false;


};

IDEWindow::IDEWindow(QWidget *parent) :
    wMainWindow(parent),
    ui(new Ui::IDEWindow)
{
    d = new IDEWindowPrivate;
    qRegisterMetaType<QFileInfoList>("QFileInfoList");
    Subscriber::reg();
    this->regMessageIds({Type::M_OPEN_EDITOR,Type::M_OPEN_FIND,Type::M_GOTO,Type::M_OPEN_FILE_TRANSFTER});
    ui->setupUi(this);
    this->resetupUi();

    ui->toolBar->setFixedHeight(32);
    m_dockingPaneManager = new DockingPaneManager(this);
    auto w = m_dockingPaneManager->widget();
    auto workbench = m_dockingPaneManager->workbench();
    workbench->setContentsMargins(0,0,0,6);

    this->setCentralWidget(w);



    connect(m_dockingPaneManager->workbench(),&DockingWorkbench::beforePaneClose,this,&IDEWindow::onBeforePaneClose);
    connect(m_dockingPaneManager->workbench(),&DockingWorkbench::paneClosed,this,&IDEWindow::onPaneClosed);
    connect(ui->actionOpen_Project,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionOpen_Folder,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionOpen_File,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionNew_File,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionNew_Project,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionSave_S,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionSave_As_A,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionSave_All_L,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionClose_C,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionClose_Project,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionQuit_Q_tAlt_F4,&QAction::triggered,this,&IDEWindow::close);

    //Edit
    connect(ui->actionUndo_U,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionRedo,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionCut_X,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionCopy_C,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionPaste_P,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionDelete_D,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionSelect_All,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionGoto,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionFind_Replace,&QAction::triggered,this,&IDEWindow::onActionTriggered);

    //View
    connect(ui->actionResource_Manage,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionVersion_Control,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionServer,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionTerminal,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionFile_Transfer,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionLog,&QAction::triggered,this,&IDEWindow::onActionTriggered);

    connect(ui->actionAbout,&QAction::triggered,this,&IDEWindow::onDump);




    wToastManager::init(this);
    BackendThread::init()->start();
    CodeEditorManager::init(m_dockingPaneManager);


    //this->restoreFromSettings();


    this->forTest();
}

IDEWindow::~IDEWindow()
{

    this->shutdown();

    delete d;
    delete ui;

    NetworkManager::destory();
}

void IDEWindow::shutdown(){
    //save setting
    auto dockpanes = m_dockingPaneManager->toJson();
    auto projects = ResourceManagerModel::getInstance()->toJson();
    auto settings = IDESettings::getInstance(this);
    settings->setDockpanes(dockpanes);
    settings->setProjects(projects);
    settings->saveToFile();
    IDESettings::destory();

    wToastManager::destory();
    auto t = BackendThread::getInstance();
    t->stop()->quit();
    // t->wait();
    CodeEditorManager::destory();
    Subscriber::unReg();


    //destory file transfter model
    FileTransferModel::destory();


}

bool IDEWindow::onReceive(Event* e){
    if(e->id()==Type::M_OPEN_EDITOR){
        auto one = static_cast<OpenEditorData*>(e->data());
        CodeEditorManager::getInstance()->open(m_dockingPaneManager,one->path,one->line,one->column);
        e->ignore();
        return true;
    }else if(e->id()==Type::M_OPEN_FIND){
        auto one = static_cast<OpenFindData*>(e->data());
        this->onOpenFindAndReplace(one->mode,one->text,one->scope);
        return true;
    }else if(e->id()==Type::M_GOTO){
        auto lineNo = static_cast<int*>(e->data());
        auto pane = this->currentEditorPane();
        if(pane!=nullptr){
            pane->editor()->gotoLine(*lineNo);
            pane->editor()->setFocus();
        }
    }else if(e->id()==Type::M_OPEN_FILE_TRANSFTER){
        //auto data = static_cast<UploadData*>(e->data());
        auto pane = FileTransferPane::open(m_dockingPaneManager,true);
        pane->activeToCurrent();

    }
    return false;
}

void IDEWindow::onDump(){
    m_dockingPaneManager->workbench()->dump("dump");
}

void IDEWindow::onPaneClosed(QString&id,QString&group,bool isClient){
    qDebug()<<"onPaneClosed:"<<id<<";"<<group<<";"<<isClient;
}
void IDEWindow::onBeforePaneClose(DockingPane* pane,bool isClient){
    qDebug()<<"onBeforePaneClose:"<<pane<<";"<<isClient;
    if(isClient){
        //pane->setCloseEnable(false);
    }
}

void IDEWindow::onActionTriggered(){
    QObject* sender = this->sender();
    if(sender==ui->actionOpen_Project){
        OpenProjectWindow::open(this);
    }else if(sender==ui->actionOpen_Folder){
        QString dir = QFileDialog::getExistingDirectory(this, tr("Open Folder"), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        this->onOpenFolder(dir);
    }else if(sender==ui->actionOpen_File){
        QString path = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("All Files (*.*)"));
        this->onOpenFile(path);
    }else if(sender==ui->actionNew_Project){
        NewProjectWindow::open(this);
    }else if(sender==ui->actionNew_File){
        CodeEditorManager::getInstance()->open(m_dockingPaneManager,"");
    }else if(sender==ui->actionSave_S || sender==ui->actionSave_As_A){
        //save
        //save as
        DockingPaneClient* client = m_dockingPaneManager->workbench()->client();
        if(client!=nullptr){
            auto pane = client->currentPane();
            if(pane!=nullptr){
                pane->save(sender==ui->actionSave_As_A);
            }
        }
    }else if(sender==ui->actionSave_All_L){
        int count = m_dockingPaneManager->workbench()->clientCount();
        for(int i=0;i<count;i++){
            auto client = m_dockingPaneManager->workbench()->client(i);
            int paneCount = client->paneCount();
            for(int j=0;j<paneCount;j++){
                auto pane = client->pane(j);
                pane->save();
            }
        }
    }else if(sender==ui->actionClose_C){
        DockingPaneClient* client = m_dockingPaneManager->workbench()->client();
        if(client!=nullptr){
            client->closeCurrent();
        }
    }else if(sender==ui->actionClose_Project){

        /*auto model = ResourceManagerModel::getInstance();
        if(model!=nullptr){

        }*/
        /*if(ResourceManagerPane::getInstance()==nullptr){

        }*/
        Publisher::getInstance()->post(Type::M_CLOSE_PROJECT);



    }else if(sender==ui->actionGoto){
        auto pane = this->currentEditorPane();
        if(pane!=nullptr){
            auto editor = pane->editor();
            auto cursor = editor->textCursor();
            int cur = cursor.blockNumber() + 1;
            //GotoLineDialog::open(this,cur,editor->document()->lineCount());
        }
    }else if(sender==ui->actionFind_Replace){
        this->onOpenFindAndReplace(0,{},{});
    }else if(sender==ui->actionResource_Manage){
        auto pane = ResourceManagerPane::open(m_dockingPaneManager,true);
        pane->activeToCurrent();
        auto model = ResourceManagerModel::getInstance();
        //qDebug()<<"count:"<<model->rowCount();
        if(model->rowCount()==0){
            //first open
            this->restoreProjects();
        }



    }else if(sender==ui->actionVersion_Control){
        auto pane = VersionControlPane::open(m_dockingPaneManager,true);
        pane->activeToCurrent();
    }else if(sender==ui->actionServer){
        auto pane = ServerManagePane::open(m_dockingPaneManager,true);
        pane->activeToCurrent();
    }else if(sender==ui->actionTerminal){

    }else if(sender==ui->actionFile_Transfer){
        auto pane = FileTransferPane::open(m_dockingPaneManager,true);
        pane->activeToCurrent();
    }else if(sender==ui->actionLog){

    }
}

void IDEWindow::onOpenFile(const QString& path){
    CodeEditorManager::getInstance()->open(m_dockingPaneManager,path);
}
void IDEWindow::onOpenFolder(const QString& path){
    ProjectRecord record;
    record.path = path;
    QFileInfo fi(path);
    record.name = fi.fileName();
    Publisher::getInstance()->post(Type::M_OPEN_PROJECT,(void*)&record);
}

void IDEWindow::onSearch(const QString& text,int flags,bool hightlight){
    auto pane = this->currentEditorPane();
    if(pane!=nullptr){
        auto editor = pane->editor();
        editor->findText(text,flags,hightlight);
    }
}

void IDEWindow::onSearchAll(const QString& text,const QString& scope,int flags,const QString& filter,const QString& exclusion){
    auto pane = FindReplacePane::open(m_dockingPaneManager,true);
    pane->activeToCurrent();
    pane->runSearch(text,scope,flags,filter,exclusion);
}

void IDEWindow::onReplaceAll(const QString& before,const QString& after,const QString& scope,int flags,const QString& filter,const QString& exclusion){
    auto pane = FindReplacePane::open(m_dockingPaneManager,true);
    pane->activeToCurrent();
    pane->runReplace(before,after,scope,flags,filter,exclusion);
}

void IDEWindow::onReplace(const QString& before,const QString& after,int flags,bool hightlight){
    auto pane = this->currentEditorPane();
    if(pane!=nullptr){
        auto editor = pane->editor();
        editor->replaceText(before,after,flags,hightlight);
    }
}

void IDEWindow::onSearchCancel(){
    auto pane = this->currentEditorPane();
    if(pane!=nullptr){
        auto editor = pane->editor();
        editor->clearHighlights();
    }
}

void IDEWindow::onOpenFindAndReplace(int mode,const QString& text,const QString& scope){
    auto dialog = FindReplaceDialog::getInstance();
    if(dialog==nullptr){
        dialog = FindReplaceDialog::open(this);
        connect(dialog,&FindReplaceDialog::search,this,&IDEWindow::onSearch);
        connect(dialog,&FindReplaceDialog::searchAll,this,&IDEWindow::onSearchAll);
        connect(dialog,&FindReplaceDialog::cancelSearch,this,&IDEWindow::onSearchCancel);
        connect(dialog,&FindReplaceDialog::replace,this,&IDEWindow::onReplace);
        connect(dialog,&FindReplaceDialog::replaceAll,this,&IDEWindow::onReplaceAll);
    }else{
        dialog->show();
    }
    dialog->setMode(mode);//find
    if(!text.isEmpty()){
        dialog->setFindText(text);
    }
    if(!scope.isEmpty()){
        dialog->setSearchScope(scope);
    }
}


CodeEditorPane* IDEWindow::currentEditorPane(){
    auto client = m_dockingPaneManager->workbench()->client();
    auto pane = client->currentPane();
    if(pane!=nullptr){
        QString name = pane->metaObject()->className();
        if(name=="ady::CodeEditorPane"){
            return static_cast<CodeEditorPane*>(pane);
        }
    }
    return nullptr;
}

void IDEWindow::showEvent(QShowEvent* e){
    wMainWindow::showEvent(e);
    if(d->init==false){
        this->restoreFromSettings();
        d->init = true;
    }
}

void IDEWindow::restoreFromSettings(){
    auto settings = IDESettings::getInstance(this);
    if(settings->readFromFile()){
        if(settings->isMaximized()){
            this->showMaximized();
        }else{
            this->resize(settings->width(),settings->height());
        }
        this->restoreDockpanes();
        this->restoreProjects();
    }else{
        //start up by default
        m_dockingPaneManager->initClient();//init client container
        this->showMaximized();
        ResourceManagerPane::open(m_dockingPaneManager);
    }
}

void IDEWindow::restoreDockpanes(){
    auto settings = IDESettings::getInstance(this);
    QJsonObject dockpanes = settings->dockpanes();
    QJsonValue innerV = dockpanes.take("inner");
    if(innerV.isObject()){
        QJsonObject inner = innerV.toObject();
        int orientation = inner.take("orientation").toInt(1);
        QJsonArray list = inner.take("list").toArray();
        if(list.size()>0){
            auto root = m_dockingPaneManager->layout()->rootItem();
            this->restoreContainers(list,orientation,root);
            root->calculateStretch();
        }
    }
}

void IDEWindow::restoreContainers(QJsonArray& list,int orientation,DockingPaneLayoutItemInfo* parent){
    for(auto one:list){
        if(one.isObject()){
            QJsonObject containerJson = one.toObject();
            if(containerJson.find("tabs")!=containerJson.end()){
                int active = containerJson.find("active")->toInt(0);
                int client = containerJson.find("client")->toInt(0);
                int stretch = containerJson.find("stretch")->toDouble(0);
                int size = containerJson.find("size")->toInt(0);
                QJsonArray tabs = containerJson.take("tabs").toArray();
                //auto info = new DockingPaneLayoutItemInfo();
                auto workbench = m_dockingPaneManager->workbench();
                DockingPaneContainer* container = nullptr;
                if(client>0){
                    container = new DockingPaneClient(workbench,true);
                }else{
                    container = new DockingPaneContainer(workbench);
                }
                DockingPaneLayoutItemInfo* info = nullptr;
                if(orientation==DockingPaneLayoutItemInfo::Vertical){
                    info = parent->insertItem(workbench,new QWidgetItem(container),DockingPaneManager::Bottom);
                }else{
                    info = parent->insertItem(workbench,new QWidgetItem(container),DockingPaneManager::Right);
                }
                int num = this->restoreTabs(tabs,info);
                if(num>0){
                    //info->setStretch(stretch);
                    info->setManualSize(size);
                    container->setPane(num-1<active?0:active);
                    //qDebug()<<"restoreContainers"<<container<<active<<num;
                }
            }else if(containerJson.find("children")!=containerJson.end()){

                QJsonArray children = containerJson.take("children").toArray();
                if(children.size()>1){
                    int stretch = containerJson.find("stretch")->toDouble(0);
                    int size = containerJson.find("size")->toInt(0);
                    auto info = new DockingPaneLayoutItemInfo(nullptr,DockingPaneManager::Left,parent);//create empty layout item
                    info->initHandle(m_dockingPaneManager->workbench());//init drag handle
                    int ori = orientation==DockingPaneLayoutItemInfo::Horizontal?DockingPaneLayoutItemInfo::Vertical:DockingPaneLayoutItemInfo::Horizontal;
                    this->restoreContainers(children,ori,info);
                    parent->appendItem(info);
                    info->setManualSize(size);
                    //qDebug()<<"info"<<info<<info->geometry(info->spacing());
                    info->calculateStretch();
                    //info->setStretch(stretch);
                }
            }
        }
    }
}

int IDEWindow::restoreTabs(QJsonArray& list,DockingPaneLayoutItemInfo* info){
    int total = 0;
    for(auto one:list){
        if(one.isObject()){
            auto tab = one.toObject();
            auto container = info->container();
            const QString group = tab.take("group").toString();
            QJsonObject data = tab.take("data").toObject();
            auto pane = PaneLoader::init(m_dockingPaneManager,group,data);
            if(pane!=nullptr){
                container->appendPane(pane);
                if(total==0){
                    //set object name
#ifdef Q_DEBUG
                    container->setObjectName(pane->id()+"_container");
                    info->setObjectName(pane->id()+"_container_itemInfo");
#endif
                }
                total += 1;
            }
        }
    }
    return total;
}

void IDEWindow::restoreProjects(){
    //ProjectStorage projectStorage
    //27 demo
    //7 svn
    //1 guzheng
    auto settings = IDESettings::getInstance(this);
    QJsonArray projects = settings->projects();
    for(auto one:projects){
        if(one.isObject()){
            Publisher::getInstance()->post(Type::M_OPEN_PROJECT,one);
        }
    }
}



void IDEWindow::forTest(){
    //Css::Scanner scanner;
    Html::Scanner scanner;
    const QString filename = "D:/wamp/www/test/index.html";
    //CodeEditorManager::getInstance()->open(filename);
    /*QFile fi(filename);
    if(fi.open(QIODevice::ReadOnly)){
        int state = Css::Scanner::Normal;
        while (!fi.atEnd()) {
           const QString text = fi.readLine();
            int from = 0;
            auto list = scanner(from,text,state);
            qDebug()<<"size:"<<list.size();
            Html::Scanner::dump(text,list);
            state = scanner.state();
            //break;
        }
        fi.close();
    }else{
        qDebug()<<"file not opened";
    }*/
}



}
