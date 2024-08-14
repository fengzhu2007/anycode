#include "idewindow.h"
#include "ui_idewindow.h"
#include "docking_pane_manager.h"
#include "docking_workbench.h"
#include "docking_pane.h"
#include "docking_pane_client.h"
#include "project/open_project_window.h"
#include "project/new_project_window.h"
#include "panes/resource_manager/resource_manager_pane.h"
#include "panes/code_editor/code_editor_pane.h"
#include "panes/code_editor/code_editor_manager.h"
#include "panes/code_editor/goto_line_dialog.h"
#include "panes/version_control/version_control_pane.h"
#include "panes/server_manage/server_manage_pane.h"
#include "panes/file_transfer/file_transfer_pane.h"
#include "panes/find_replace/find_replace_dialog.h"
#include "panes/find_replace/find_replace_pane.h"

#include "core/event_bus/event.h"
#include "core/event_bus/publisher.h"
#include "core/event_bus/type.h"
#include "core/event_bus/event_data.h"
#include "core/backend_thread.h"
#include "storage/project_storage.h"

/*#include "highlighter.h"
#include "texteditorsettings.h"
#include "core/editormanager.h"
#include "core/actionmanager/actionmanager.h"
#include "codeassist/codeassistant.h"
#include "codeassist/documentcontentcompletion.h"
#include "snippets/snippetprovider.h"*/

#include "w_toast.h"
#include "network/network_manager.h"

#include <QLabel>
#include <QCloseEvent>
#include <QFileDialog>
#include <QTextCodec>
#include <QDebug>

namespace ady{
IDEWindow::IDEWindow(QWidget *parent) :
    wMainWindow(parent),
    ui(new Ui::IDEWindow)
{
    qRegisterMetaType<QFileInfoList>("QFileInfoList");
    Subscriber::reg();
    this->regMessageIds({Type::M_OPEN_EDITOR,Type::M_OPEN_FIND,Type::M_GOTO});
    ui->setupUi(this);
    this->resetupUi();

    ui->toolBar->setFixedHeight(32);
    m_dockingPaneManager = new DockingPaneManager(this);
    auto w = m_dockingPaneManager->widget();
    m_dockingPaneManager->workbench()->setContentsMargins(0,0,0,6);
    this->setCentralWidget(w);

    ResourceManagerPane::open(m_dockingPaneManager);

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


    wToastManager::init(this);
    BackendThread::init()->start();
    CodeEditorManager::init(m_dockingPaneManager);
    CodeEditorManager::getInstance()->open("D:/wamp/www/demo.php");

    //ProjectStorage projectStorage
    //27 demo
    //7 svn
    //1 guzheng
    ProjectRecord record = ProjectStorage().one(27);
    Publisher::getInstance()->post(Type::M_OPEN_PROJECT,(void*)&record);

    //auto widget = new QPlainTextEdit(m_dockingPaneManager->widget());
    //m_dockingPaneManager->createPane("test","test","TEST",widget,DockingPaneManager::Center);

    /*new TextEditor::TextEditorSettings();
    new Core::ActionManager(this);
    auto editor = new TextEditor::TextEditorWidget(m_dockingPaneManager->widget());
    TextEditor::SnippetProvider::registerGroup(TextEditor::Constants::TEXT_SNIPPET_GROUP_ID,tr("Text", "SnippetProvider"));
    static TextEditor::DocumentContentCompletionProvider basicSnippetProvider;
    QSharedPointer<TextEditor::TextDocument> doc(new TextEditor::TextDocument(Core::Constants::K_DEFAULT_TEXT_EDITOR_ID));
    QString error;
    doc->setCodec(QTextCodec::codecForName("UTF-8"));
    doc->open(&error,Utils::FilePath::fromString("D:/wamp/www/guzheng2018/phpcms/base.php"),Utils::FilePath::fromString("D:/wamp/www/guzheng2018/phpcms/base.php"));
    editor->setTextDocument(doc);
    editor->configureGenericHighlighter();
    doc->setCompletionAssistProvider(&basicSnippetProvider);
    m_dockingPaneManager->createPane("test","test","TEST",editor,DockingPaneManager::Center);*/
}

IDEWindow::~IDEWindow()
{
    wToastManager::destory();
    auto t = BackendThread::getInstance();
    t->stop()->quit();
   // t->wait();
    CodeEditorManager::destory();
    Subscriber::unReg();
    delete ui;

    NetworkManager::destory();
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
    }else if(sender==ui->actionResource_Manage){
        auto pane = ResourceManagerPane::open(m_dockingPaneManager,true);
        pane->activeToCurrent();
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

/*void IDEWindow::closeEvent(QCloseEvent *event){

    event->accept();
}*/


}
