#include "idewindow.h"
#include "ui_idewindow.h"
#include <docking_pane_manager.h>
#include <docking_workbench.h>
#include <docking_pane.h>
#include <docking_pane_client.h>
#include <docking_pane_tabbar.h>
#include "project/open_project_window.h"
#include "project/new_project_window.h"
#include "panes/resource_manager/resource_manager_pane.h"
#include "panes/resource_manager/resource_manager_model.h"
#include "panes/code_editor/code_editor_pane.h"
#include "panes/code_editor/code_editor_manager.h"
#include "panes/code_editor/goto_line_dialog.h"
#include "panes/version_control/version_control_pane.h"
#include "panes/server_manage/server_manage_pane.h"
#include "panes/file_transfer/file_transfer_pane.h"
#include "panes/find_replace/find_replace_dialog.h"
#include "panes/find_replace/find_replace_pane.h"
#include "panes/file_transfer/file_transfer_model.h"
#include "panes/terminal/terminal_pane.h"
#include "panes/output/output_pane.h"
#include "panes/notification/notification_pane.h"
#include "panes/db/dbms_pane.h"

#include "panes/loader.h"
#include "core/event_bus/event.h"
#include "core/event_bus/publisher.h"
#include "core/event_bus/type.h"
#include "core/event_bus/event_data.h"
#include "core/backend_thread.h"
#include "core/layout_settings.h"
#include "core/schedule/schedule.h"
//#include "core/extension/extension_engine.h"
#include "modules/options/options_settings.h"
#include "modules/options/environment_settings.h"
#include "storage/project_storage.h"
#include "storage/recent_storage.h"
#include "network/network_manager.h"


#include "languages/html/htmlscanner.h"

#include "modules/help/about_dialog.h"
//#include "modules/help/update_dialog.h"
#include "modules/options/options_dialog.h"
#include "modules/addon/addon_manager_dialog.h"
#include "modules/import_export/import_export_dialog.h"

#include "tools/ssl_query/ssl_query_dialog.h"

#include "components/statusbar/status_bar_view.h"
#include "common.h"
//#include "app_oss.h"
#include "core/theme.h"

#include <w_toast.h>
#include <w_window.h>


#include <QLabel>
#include <QCloseEvent>
#include <QFileDialog>
#include <QTextCodec>
#include <QProcess>
#include <QDesktopServices>
#include <QLayout>
#include <QFontDatabase>
#include <QShortcut>
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

    //this->setStyleSheet(AppOSS::global());
    //qDebug()<<Theme::getInstance()->qss();
    auto theme = Theme::getInstance();
    this->setStyleSheet(theme->qss());
    d = new IDEWindowPrivate;
    qRegisterMetaType<QFileInfoList>("QFileInfoList");
    Subscriber::reg();
    this->regMessageIds({Type::M_OPEN_EDITOR,Type::M_OPEN_FIND,Type::M_GOTO,Type::M_OPEN_FILE_TRANSFTER,Type::M_OPEN_PANE,Type::M_RESTART,
Type::M_TOGGLE_NOTIFICATION,Type::M_OPEN_TERMINAL});
    ui->setupUi(this);

    this->setWindowIcon(QIcon(":/Resource/images/logo.icns"));



    //ui->statusbar->setFixedHeight(25);
    ui->statusbar->addWidget(StatusBarView::make(ui->statusbar),1);
    ui->statusbar->setContentsMargins({0,0,0,0});
    //qDebug()<<"status bar layout"<<ui->statusbar->layout()->margin();
    ui->statusbar->layout()->setMargin(0);
    ui->statusbar->layout()->setSpacing(0);


    this->resetupUi();

    ui->toolBar->setFixedHeight(32);
    m_dockingPaneManager = new DockingPaneManager(this,theme->style()==Theme::Light?DockingTheme::Light:DockingTheme::Dark);
    auto w = m_dockingPaneManager->widget();
    auto workbench = m_dockingPaneManager->workbench();
    workbench->setContentsMargins(0,0,0,6);

    this->setCentralWidget(w);



    //connect(m_dockingPaneManager->workbench(),&DockingWorkbench::beforePaneClose,this,&IDEWindow::onBeforePaneClose);
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
    connect(ui->actionClear,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionClear_Files,&QAction::triggered,this,&IDEWindow::onActionTriggered);

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
    connect(ui->actionAuto_Format,&QAction::triggered,this,&IDEWindow::onActionTriggered);

    //View
    connect(ui->actionResource_Manage,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionVersion_Control,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionServer,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionDatabase,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionTerminal,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionFile_Transfer,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionOutput,&QAction::triggered,this,&IDEWindow::onActionTriggered);


    //tool

    connect(ui->actionSSL_Querier,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionOptions,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionImport_And_Export,&QAction::triggered,this,&IDEWindow::onActionTriggered);


    //addon
    connect(ui->actionAddon_Manage,&QAction::triggered,this,&IDEWindow::onActionTriggered);

    //help
    connect(ui->actionHome,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionView_Help_H,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionDonate,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionUpdate,&QAction::triggered,this,&IDEWindow::onActionTriggered);
    connect(ui->actionAbout,&QAction::triggered,this,&IDEWindow::onActionTriggered);

    //debug
    connect(ui->actionDebug,&QAction::triggered,this,&IDEWindow::onActionTriggered);


    //this->forTest();
    this->initView();
    this->boot();
}


void IDEWindow::initView(){
    //init menu
    connect(ui->menuRecent_Files,&QMenu::aboutToShow,this,&IDEWindow::onRecentMenuShow);
    connect(ui->menuRecent_Projects,&QMenu::aboutToShow,this,&IDEWindow::onRecentMenuShow);

    //init command line
#ifdef Q_OS_WIN
    QMenu* commandLineMenu = new QMenu(ui->menuTool_T);
    commandLineMenu->setTitle(tr("Command Line"));
    auto icon = QIcon(":/Resource/icons/ImmediateWindow_16x.svg");
    commandLineMenu->setIcon(icon);
    commandLineMenu->addAction(icon,tr("Command Prompt"),[this]{
        QStringList arguments;
        arguments << "/c" << "start" << "cmd.exe";
        this->runExe("cmd.exe",arguments);
    });
    commandLineMenu->addAction(QIcon(":/Resource/icons/PowerShellInteractiveWindow_16x.svg"),tr("PowerShell"),[this]{
        QStringList arguments;
        arguments << "/c" << "start" << "powershell.exe";
        this->runExe("powershell.exe",arguments);
    });
    ui->menuTool_T->insertMenu(ui->actionOptions,commandLineMenu);
    ui->menuTool_T->insertSeparator(ui->actionOptions);

#elif defined(Q_OS_MAC)

    ui->menuTool_T->addAction(QIcon(":/Resource/icons/ImmediateWindow_16x.svg"),tr("Terminal"),[this](){
        QStringList arguments;
        arguments <<QString("-a")<<QString("Terminal");
        this->runExe("open",arguments);
    });

#elif defined(Q_OS_LINUX)
    QStringList arguments;
    arguments << QString("--working-directory=%1").arg("./");
    this->runExe("gnome-terminal",arguments);
#endif
}

IDEWindow::~IDEWindow()
{

    delete d;
    delete ui;
    NetworkManager::destory();
}


void IDEWindow::boot(){
    //OptionsSettings::init();//init options settings
    wToastManager::init(this);
    BackendThread::init()->start();
    CodeEditorManager::init(m_dockingPaneManager);
    Schedule::init(this);//init Schedule
    //init js engine
    //ExtensionEngine::init(this);
}

void IDEWindow::delayBoot(){
    //add font
    QFontDatabase::addApplicationFont(":/Resource/fonts/SourceCodePro-Regular.ttf");

    this->restoreFromSettings();

    //call from show event
    //init Schedule task
    auto settings = OptionsSettings::getInstance();
    auto env = settings->environmentSettings();
    auto schedule = Schedule::getInstance();
    if(env.m_autoSave){
        schedule->addFileAutoSave(env.m_autoSaveInterval * 60 * 1000);
    }
    //add network auto close
    schedule->addNetworkAutoClose(5*60 * 1000);
    //add network status watching
    schedule->addNetworkStatusWatching(3 * 1000);
    //add ssl query notifiy
    schedule->addSSLQuery();


    Schedule::start();
}

void IDEWindow::shutdown(){
    //save setting
    Schedule::stop();

    LayoutSettings::destory();

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
        //auto one = static_cast<OpenEditorData*>(e->data());
        auto one = e->toJsonOf<OpenEditorData>().toObject();
        const QString path = one.find("path")->toString();
        if(!path.isEmpty()){
            /*CodeEditorManager::getInstance()->open(m_dockingPaneManager,path,one.find("line")->toInt(0),
                    one.find("column")->toInt(0));*/
            auto instance = CodeEditorManager::getInstance();
            auto pane = instance->open(path);
            auto editor = pane->editor();
            if(editor){
                auto line = one.find("line")->toInt(0);
                auto column = one.find("column")->toInt(0);
                editor->gotoLine(line,column);
                editor->setFocus();
            }
            if(one.find("locate")->toBool(false)){
                Publisher::getInstance()->post(Type::M_RESOURCE_LOCATION);
            }
            e->ignore();
            return true;
        }
    }else if(e->id()==Type::M_OPEN_FIND){
        auto one = e->toJsonOf<OpenFindData>().toObject();
        int mode = one.find("mode")->toInt(0);
        //auto one = static_cast<OpenFindData*>(e->data());
        this->onOpenFindAndReplace(mode,one.find("text")->toString(),one.find("scope")->toString());
        return true;
    }else if(e->id()==Type::M_GOTO){
        auto lineNo = static_cast<int*>(e->data());
        auto pane = this->currentEditorPane();
        if(pane!=nullptr){
            pane->editor()->gotoLine(*lineNo);
            pane->editor()->setFocus();
        }
    }else if(e->id()==Type::M_OPEN_FILE_TRANSFTER){
        auto pane = FileTransferPane::open(m_dockingPaneManager,true);
        pane->activeToCurrent();
    }else if(e->id()==Type::M_OPEN_PANE){
        auto one = static_cast<QString*>(e->data());
        PaneLoader::open(m_dockingPaneManager,*one,{});
    }else if(e->id()==Type::M_RESTART){
        //qDebug()<<"restart";
        this->restart();
    }else if(e->id()==Type::M_TOGGLE_NOTIFICATION){
        auto instance = NotificationPane::getInstance();
        if(instance){
            //close
            //instance->close();
        }else{
            //show
            auto pane = NotificationPane::open(m_dockingPaneManager,true);
            pane->activeToCurrent();
        }
    }else if(e->id()==Type::M_OPEN_TERMINAL){
        auto instance = TerminalPane::getInstance();
        if(instance==nullptr){
            //first open terminal
            auto workingDir = static_cast<QString*>(e->data());
            QJsonObject data = {
                {"currentPath",*workingDir},
            };
            auto pane = TerminalPane::open(m_dockingPaneManager,true,data);
            pane->activeToCurrent();
        }
    }
    return false;
}


void IDEWindow::runExe(const QString& executable,const QStringList& arguments){
    QProcess::startDetached(executable, arguments);
}

void IDEWindow::openUrl(const QString& url){
     QDesktopServices::openUrl(QUrl(url));
}

void IDEWindow::restart(){
    QString program = QCoreApplication::applicationFilePath();
    QProcess::startDetached(program, QStringList());
    QCoreApplication::quit();
}

void IDEWindow::onDump(){
    m_dockingPaneManager->workbench()->dump("dump");
}

void IDEWindow::onPaneClosed(QString&id,QString&group,bool isClient){
    //qDebug()<<"onPaneClosed:"<<id<<";"<<group<<";"<<isClient;
}
void IDEWindow::onBeforePaneClose(DockingPane* pane,bool isClient){
    /*qDebug()<<"onBeforePaneClose:"<<pane<<";"<<isClient;
    if(isClient){
        //pane->setCloseEnable(false);
    }*/
}

void IDEWindow::onActionTriggered(){
    QObject* sender = this->sender();
    if(sender==ui->actionOpen_Project){
        OpenProjectWindow::open(this);
    }else if(sender==ui->actionOpen_Folder){
        QString dir = QFileDialog::getExistingDirectory(this, tr("Select Folder"), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        this->onOpenFolder(dir);
    }else if(sender==ui->actionOpen_File){
        QString path = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("All Files (*.*)"));
        this->onOpenFile(path);
    }else if(sender==ui->actionNew_Project){
        NewProjectWindow::open(this);
    }else if(sender==ui->actionNew_File){
        CodeEditorManager::getInstance()->open("");
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
        Publisher::getInstance()->post(Type::M_CLOSE_PROJECT);
    }else if(sender==ui->actionClear){
        RecentStorage().clear(RecentStorage::ProjectAndFolder);
    }else if(sender==ui->actionClear_Files){
        RecentStorage().clear(RecentStorage::File);

    //edit
    }else if(sender==ui->actionGoto){
        auto pane = this->currentEditorPane();
        if(pane!=nullptr){
            auto editor = pane->editor();
            auto cursor = editor->textCursor();
            int cur = cursor.blockNumber() + 1;
            GotoLineDialog::open(this,cur,editor->document()->lineCount());
        }
    }else if(sender==ui->actionFind_Replace){
        this->onOpenFindAndReplace(0,{},{});
    }else if(sender==ui->actionCopy_C){
        auto pane = this->currentEditorPane();
        if(pane!=nullptr){
            pane->doAction(DockingPane::Copy);
        }
    }else if(sender==ui->actionRedo){
        auto pane = this->currentEditorPane();
        if(pane!=nullptr){
            pane->doAction(DockingPane::Redo);
        }
    }else if(sender==ui->actionUndo_U){
        auto pane = this->currentEditorPane();
        if(pane!=nullptr){
            pane->doAction(DockingPane::Undo);
        }
    }else if(sender==ui->actionPaste_P){
        auto pane = this->currentEditorPane();
        if(pane!=nullptr){
            pane->doAction(DockingPane::Paste);
        }
    }else if(sender==ui->actionDelete_D){
        auto pane = this->currentEditorPane();
        if(pane!=nullptr){
            pane->doAction(DockingPane::Delete);
        }
    }else if(sender==ui->actionCut_X){
        auto pane = this->currentEditorPane();
        if(pane!=nullptr){
            pane->doAction(DockingPane::Cut);
        }
    }else if(sender==ui->actionAuto_Format){
        qDebug()<<"auto format";
        auto pane = this->currentEditorPane();
        if(pane!=nullptr){
            auto editor = pane->editor();
            if(editor){
                editor->autoFormat();
            }
        }
    }else if(sender==ui->actionSelect_All){
        auto pane = this->currentEditorPane();
        if(pane!=nullptr){
            pane->doAction(DockingPane::SelectAll);
        }
    }
    //view
    else if(sender==ui->actionResource_Manage){
        auto pane = ResourceManagerPane::open(m_dockingPaneManager,true);
        pane->activeToCurrent();
        auto model = ResourceManagerModel::getInstance();
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
    }else if(sender==ui->actionDatabase){
        auto pane = DBMSPane::open(m_dockingPaneManager,true);
        pane->activeToCurrent();
    }else if(sender==ui->actionTerminal){
        auto pane = TerminalPane::open(m_dockingPaneManager,true);
        pane->activeToCurrent();
    }else if(sender==ui->actionFile_Transfer){
        auto pane = FileTransferPane::open(m_dockingPaneManager,true);
        pane->activeToCurrent();
    }else if(sender==ui->actionOutput){
        auto pane = OutputPane::open(m_dockingPaneManager,true);
        pane->activeToCurrent();

    //tool
    }else if(sender==ui->actionSSL_Querier){
        SSLQueryDialog::open(this);
    }else if(sender==ui->actionOptions){
        OptionsDialog::open(this);
    }else if(sender==ui->actionImport_And_Export){
        ImportExportDialog::open(this);
    //addon
    }else if(sender==ui->actionAddon_Manage){
        AddonManagerDialog::open(this);
    //help
    }else if(sender==ui->actionView_Help_H){
        this->openUrl(APP_HELP_URL);
    }else if(sender==ui->actionHome){
        this->openUrl(APP_URL);
    }else if(sender==ui->actionDonate){
        this->openUrl(APP_DONATE_URL);
    }else if(sender==ui->actionUpdate){
        //UpdateDialog::open(this);
        this->openUrl(APP_UPDATE_URL);
    }else if(sender==ui->actionAbout){
        AboutDialog::open(this);
    }else if(sender==ui->actionDebug){
        //m_dockingPaneManager->dump();
#ifdef Q_DEBUG

#endif
        //ExtensionEngine::run(":/Resource/extension/test.js");
        //QuickPane::open(m_dockingPaneManager,true);
        //auto w = new wWindow(this);
        //w->setMaximumSize({-1,-1});
        //w->resize({400,300});
        //w->show();
    }
}

void IDEWindow::onOpenFile(const QString& path){
    CodeEditorManager::getInstance()->open(path);
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
        if(editor){
            auto ret = editor->findText(text,flags,hightlight);
            if(!ret){
                wToast::showText(tr("Text \"%1\" not found").arg(text));
            }
        }
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
    }else{
        dialog->setSearchScope(":");//set current document scope
    }
}

void IDEWindow::onRecentMenuShow(){
    auto sender = static_cast<QMenu*>(this->sender());
    if(sender==ui->menuRecent_Files){
        static QList<QAction*> actionFileList;
        if(actionFileList.size()>0){
            //remove action
            for(auto one:actionFileList){
                sender->removeAction(one);
            }
        }
        auto list = RecentStorage().list(RecentStorage::File);
        int size = list.size();
        if(size>0){
            auto separator = sender->insertSeparator(ui->actionClear_Files);
            actionFileList<<separator;
            for(auto one:list){
                const QString path = one.path;
                auto action = new QAction(path,sender);
                actionFileList<<action;
                connect(action,&QAction::triggered,this,[this,path](){
                    this->onOpenFile(path);
                });
                sender->insertAction(separator,action);
            }
        }
    }else if(sender==ui->menuRecent_Projects){
        static QList<QAction*> actionProjectList;
        if(actionProjectList.size()>0){
            //remove action
            for(auto one:actionProjectList){
                sender->removeAction(one);
            }
        }
        auto list = RecentStorage().list(RecentStorage::ProjectAndFolder);
        int size = list.size();
        if(size>0){
            auto separator = sender->insertSeparator(ui->actionClear);
            actionProjectList<<separator;
            for(auto one:list){
                const QString path = one.path;
                auto action = new QAction(path,sender);
                actionProjectList<<action;
                connect(action,&QAction::triggered,this,[this,one](){
                    if(one.dataid>0){
                        auto proj = ProjectStorage().one(one.dataid);
                        if(proj.id>0){
                            this->openProject(proj);
                            return ;
                        }
                    }
                    this->onOpenFolder(one.path);
                });
                sender->insertAction(separator,action);
            }
        }
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

        d->init = true;
        this->delayBoot();
    }
}

void IDEWindow::closeEvent(QCloseEvent* e){
    auto dockpanes = m_dockingPaneManager->toJson();
    auto projects = ResourceManagerModel::getInstance()->toJson();
    auto settings = LayoutSettings::getInstance(this);
    settings->setDockpanes(dockpanes);
    settings->setProjects(projects);
    if(!m_dockingPaneManager->close()){
        e->ignore();
        return ;
    }
    settings->saveToFile();
    this->shutdown();
}



void IDEWindow::restoreFromSettings(){
    auto settings = LayoutSettings::getInstance(this);
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

    //init client
    auto callback = [](QDropEvent* e){
        if (e->mimeData()->hasUrls()) {
            QList<QUrl> urls = e->mimeData()->urls();
            for(auto url:urls){
                QString path =  url.toLocalFile();
                QFileInfo fi(path);
                if(fi.isFile()){
                    auto data = OpenEditorData{path,0,0,false};
                    Publisher::getInstance()->post(Type::M_OPEN_EDITOR,&data);
                }
            }
        }
    };
    auto workbench = m_dockingPaneManager->workbench();
    auto clientCount = workbench->clientCount();
    for(auto i=0;i<clientCount;i++){
        auto client = workbench->client(i);
        client->setDropCallback(callback);
    }

}

void IDEWindow::restoreDockpanes(){
    auto settings = LayoutSettings::getInstance(this);
    QJsonObject dockpanes = settings->dockpanes();
    m_dockingPaneManager->restore(dockpanes,PaneLoader::init);
}


void IDEWindow::restoreProjects(){
    auto settings = LayoutSettings::getInstance(this);
    QJsonArray projects = settings->projects();
    for(auto one:projects){
        if(one.isObject()){
            Publisher::getInstance()->post(Type::M_OPEN_PROJECT,one);
        }
    }
}

void IDEWindow::openProject(ProjectRecord& proj){
    Publisher::getInstance()->post(new Event(Type::M_OPEN_PROJECT,&proj));
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
