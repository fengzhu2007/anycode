#include "code_editor_manager.h"
#include "code_editor_pane.h"
#include "docking_pane_manager.h"
#include "docking_pane_container.h"
#include "core/event_bus/event.h"
#include "core/event_bus/type.h"
#include "components/message_dialog.h"


#include "texteditorsettings.h"
//#include "core/actionmanager/actionmanager.h"
#include "codeassist/documentcontentcompletion.h"
//#include "snippets/snippetprovider.h"
#include "displaysettings.h"


#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>
#include <QThread>
#include <QFileSystemWatcher>
#include <QTextCodec>
#include <QAction>
#include <QDesktopServices>
#include <QApplication>
#include <QClipboard>
#include <QDebug>
namespace ady{




CodeEditorManager* CodeEditorManager::instance = nullptr;

class CodeEditorManagerPrivate{
public:
    QList<CodeEditorPane*> list;
    QMutex mutex;
    DockingPaneManager* docking_manager;
    QFileSystemWatcher *watcher;
    TextEditor::TextEditorSettings* settings;
    //Core::ActionManager* action_manager;
    //QSharedPointer<TextEditor::TextDocument> doc;
    //std::unique_ptr<TextEditor::TextEditorActionHandler> m_textEditorActionHandler;
    TextEditor::DocumentContentCompletionProvider provider;
    CodeEditorPane* current;
    CodeEditorView* editor;


    QAction *undoAction = nullptr;
    QAction *redoAction = nullptr;
    QAction *copyAction = nullptr;
    QAction *copyHtmlAction = nullptr;
    QAction *cutAction = nullptr;
    QAction *indentAction = nullptr;
    QAction *autoFormatAction = nullptr;
    QAction *addOrRemoveCommentAction = nullptr;
    QAction *pasteAction = nullptr;
    QAction *selectAllAction=nullptr;
    QAction *textWrappingAction = nullptr;
    QAction *visualizeWhitespaceAction = nullptr;

    /*QAction *autoFormatAction = nullptr;
    QAction *m_visualizeWhitespaceAction = nullptr;
    QAction *m_textWrappingAction = nullptr;
    QAction *m_unCommentSelectionAction = nullptr;
    QAction *m_unfoldAllAction = nullptr;
    QAction *m_followSymbolAction = nullptr;
    QAction *m_followSymbolInNextSplitAction = nullptr;
    QAction *m_findUsageAction = nullptr;
    QAction *m_renameSymbolAction = nullptr;
    QAction *m_jumpToFileAction = nullptr;
    QAction *m_jumpToFileInNextSplitAction = nullptr;
    QList<QAction *> m_modifyingActions;*/


    QAction* saveAction=nullptr;
    QAction* closeAction=nullptr;
    QAction* closeOtherAction=nullptr;
    QAction* closeAllAction=nullptr;
    QAction* copyPathAction=nullptr;
    QAction* openFolderAction=nullptr;
    QAction* floatTabAction=nullptr;









};

CodeEditorManager::CodeEditorManager(DockingPaneManager* docking_manager)
{
    Subscriber::reg();
    this->regMessageIds({Type::M_WILL_RENAME_FILE,Type::M_RENAMED_FILE,Type::M_WILL_RENAME_FOLDER,Type::M_RENAMED_FOLDER,Type::M_RELOAD_FILE,Type::M_DELETE_FILE,Type::M_DELETE_FOLDER});
    d = new CodeEditorManagerPrivate;
    d->docking_manager = docking_manager;
    d->watcher = new QFileSystemWatcher(this);
    connect(d->watcher,&QFileSystemWatcher::fileChanged,this,&CodeEditorManager::onFileChanged);

    //init texteditor evn
    //d->m_textEditorActionHandler.reset(new TextEditor::TextEditorActionHandler(Core::Constants::K_DEFAULT_TEXT_EDITOR_ID,Core::Constants::K_DEFAULT_TEXT_EDITOR_ID,TextEditor::TextEditorActionHandler::Format|TextEditor::TextEditorActionHandler::UnCommentSelection |TextEditor::TextEditorActionHandler::UnCollapseAll|TextEditor::TextEditorActionHandler::FollowSymbolUnderCursor));
    //init theme
    //Constants::DEFAULT_DARK_THEME
    d->settings = new TextEditor::TextEditorSettings();
    TextEditor::TextEditorEnvironment::init();
    /*Utils::Theme *theme = new Utils::Theme(Core::Constants::DEFAULT_DARK_THEME);
    Utils::setCreatorTheme(theme);
    d->settings = new TextEditor::TextEditorSettings();
    //d->action_manager = new Core::ActionManager(this);
    TextEditor::SnippetProvider::registerGroup(TextEditor::Constants::TEXT_SNIPPET_GROUP_ID,tr("Text", "SnippetProvider"));*/

    //d->doc = QSharedPointer<TextEditor::TextDocument>(new TextEditor::TextDocument(Core::Constants::K_DEFAULT_TEXT_EDITOR_ID));
    //d->doc->setCodec(QTextCodec::codecForName("UTF-8"));
    //d->doc->setCompletionAssistProvider(&d->provider);

    /*QString error;
    doc->setCodec(QTextCodec::codecForName("UTF-8"));
    doc->open(&error,Utils::FilePath::fromString("D:/wamp/www/anycode/modules/App/controller/Video.php"),Utils::FilePath::fromString("D:/wamp/www/anycode/modules/App/controller/Video.php"));
    editor->setTextDocument(doc);
    editor->configureGenericHighlighter();
    doc->setCompletionAssistProvider(&basicSnippetProvider);*/
}

CodeEditorManager::~CodeEditorManager(){
    Subscriber::unReg();
    TextEditor::TextEditorEnvironment::destory();
    delete d;
}

CodeEditorManager* CodeEditorManager::getInstance(){
    return instance;
}

CodeEditorManager* CodeEditorManager::init(DockingPaneManager* docking_manager){
    if(instance==nullptr){
        instance = new CodeEditorManager(docking_manager);
    }
    return instance;
}

void CodeEditorManager::destory(){
    if(instance!=nullptr){
        delete instance;
        instance = nullptr;
    }
}

CodeEditorPane* CodeEditorManager::get(const QString& path){
    if(path.isEmpty()){
        return nullptr;
    }
    QMutexLocker locker(&d->mutex);
    foreach(auto one,d->list){
        if(one->path()==path){
            return one;
        }
    }
    return nullptr;
}

QList<CodeEditorPane*> CodeEditorManager::getAll(const QString& prefix){
    QList<CodeEditorPane*> list;
    QMutexLocker locker(&d->mutex);
    foreach(auto one,d->list){
        if(one->path().startsWith(prefix)){
            list.push_back(one);
        }
    }
    return list;
}

CodeEditorPane* CodeEditorManager::open(DockingPaneManager* dockingManager,const QString& path,int line,int column){
    auto pane = this->get(path);
    if(pane==nullptr){
        const QJsonObject data = {{"path",path},{"line",line}};
        pane = CodeEditorPane::make(dockingManager,data);
        /*pane = new CodeEditorPane();
        if(path.isEmpty()){
            pane->setWindowTitle(QObject::tr("New File"));
        }else{
            d->watcher->addPath(path);
            QFileInfo fi(path);
            pane->setWindowTitle(fi.fileName());
            pane->readFile(path);
        }*/
        dockingManager->createPane(pane,DockingPaneManager::Center,true);
        //pane->editor()->gotoLine(1);
    }else{
        //set pane to current
        pane->activeToCurrent();
    }
    pane->editor()->gotoLine(line,column);
    pane->editor()->setFocus();
    return pane;
}

CodeEditorPane* CodeEditorManager::open(const QString& path){
    return this->open(d->docking_manager,path);
}

void CodeEditorManager::append(CodeEditorPane* pane){
    QMutexLocker locker(&d->mutex);
    d->list.append(pane);

}

void CodeEditorManager::remove(CodeEditorPane* pane){
    const QString path = pane->path();
    if(!path.isEmpty()){
        d->watcher->removePath(path);
    }
    QMutexLocker locker(&d->mutex);
    d->list.removeOne(pane);
}

bool CodeEditorManager::readFileLines(const QString&path){
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString text = in.readAll();
    this->appendToEditor(path,text);
    return true;
}

void CodeEditorManager::appendToEditor(const QString& path,const QString& text){
    auto pane = this->get(path);
    /*if(pane!=nullptr){
        int i = 0;
        while(pane->editor()->isShowed()==false && i++<100){
            QThread::msleep(2);
        }
        QMetaObject::invokeMethod(pane->editor(), "fillPlainText", Qt::AutoConnection,Q_ARG(QString, text));
    }*/
}

bool CodeEditorManager::appendWatchFile(const QString& path){
    return d->watcher->addPath(path);
}

bool CodeEditorManager::removeWatchFile(const QString& path){
    return d->watcher->removePath(path);
}

bool CodeEditorManager::rename(const QString& from,const QString& to){
    auto pane = this->get(from);
    if(pane!=nullptr){
        pane->rename(to);
        return true;
    }else{
        return false;
    }
}

bool CodeEditorManager::onReceive(Event* e){
    const QString id = e->id();
    if(id==Type::M_WILL_RENAME_FILE){
        auto data = static_cast<QString*>(e->data());
        this->removeWatchFile(*data);
        e->ignore();
        return true;
    }else if(id==Type::M_RENAMED_FILE){
        auto pair = static_cast<QPair<QString,QString>*>(e->data());
        if(this->rename(pair->first,pair->second)){
            this->appendWatchFile(pair->second);
        }
        e->ignore();
        return true;
    }else if(id==Type::M_WILL_RENAME_FOLDER){
        auto data = static_cast<QString*>(e->data());
        if(data->endsWith("/")==false){
            data->append("/");
        }
        QList<CodeEditorPane*> list = this->getAll(*data);
        foreach(auto one,list){
            this->removeWatchFile(one->path());
        }
        e->ignore();
        return true;
    }else if(id==Type::M_RENAMED_FOLDER){
        auto pair = static_cast<QPair<QString,QString>*>(e->data());
        QString prefix = pair->first;
        if(prefix.endsWith("/")==false){
            prefix += "/";
        }
        QString prefix_n = pair->second;
        if(prefix_n.endsWith("/")==false){
            prefix_n += "/";
        }
        QList<CodeEditorPane*> list = this->getAll(prefix);
        foreach(auto one,list){
            QString path = one->path().replace(prefix,prefix_n);
            one->rename(path);
            this->appendWatchFile(path);
        }
        e->ignore();
        return true;
    }else if(id==Type::M_RELOAD_FILE){
        auto data = static_cast<QString*>(e->data());
        auto pane = this->get(*data);
        if(pane!=nullptr){
            pane->readFile(*data);
        }
    }else if(id==Type::M_DELETE_FILE){
        auto data = static_cast<QString*>(e->data());
        auto pane = this->get(*data);
        if(pane!=nullptr){
            auto container = pane->container();
            if(container!=nullptr){
                container->closePane(pane,true);
            }
        }
    }else if(id==Type::M_DELETE_FOLDER){
        auto data = static_cast<QString*>(e->data());
        QList<CodeEditorPane*> list = this->getAll(*data);
        foreach(auto pane,list){
            auto container = pane->container();
            if(container!=nullptr){
                container->closePane(pane,true);
            }
        }
    }
    return false;
}

void CodeEditorManager::onFileChanged(const QString &path){
    auto pane = this->get(path);
    if(pane!=nullptr){
        //to fix
        /*QFileInfo fi(path);
        if(!fi.exists()){
            //delete
            if(MessageDialog::confirm(d->docking_manager->widget(),tr("The file \"%1\" is no longer there. \nDo you want to keep it?").arg(path))==QMessageBox::No){
                    //close pane
                auto container = pane->container();
                if(container!=nullptr){
                    container->closePane(pane);
                }
            }
        }else{
            //content update
            if(MessageDialog::confirm(d->docking_manager->widget(),tr("\"%1\" \nThis file has been modified by another program.\n Reload?").arg(path))==QMessageBox::Yes){
                auto container = pane->container();
                if(container!=nullptr){
                    container->setPane(container->indexOf(pane));
                }
                pane->readFile(path);
            }
        }*/
    }
}

void CodeEditorManager::onEditorActionTrigger(bool checked){
    auto sender = this->sender();
    if(sender==d->undoAction){
        d->editor->undo();
    }else if(sender==d->redoAction){
        d->editor->redo();
    }else if(sender==d->copyAction){
        d->editor->copy();
    }else if(sender==d->copyHtmlAction){
        d->editor->copyWithHtml();
    }else if(sender==d->cutAction){
        d->editor->cut();
    }else if(sender==d->indentAction){
        d->editor->autoIndent();
    }else if(sender==d->autoFormatAction){
        d->editor->autoFormat();
    }else if(sender==d->addOrRemoveCommentAction){
        d->editor->unCommentSelection();
    }else if(sender==d->pasteAction){
        d->editor->paste();
    }else if(sender==d->selectAllAction){
        d->editor->selectAll();
    }else if(sender==d->textWrappingAction){
        auto ds = d->editor->displaySettings();
        ds.m_textWrapping = checked;
        d->editor->setDisplaySettings(ds);
    }else if(sender==d->visualizeWhitespaceAction){
        auto ds = d->editor->displaySettings();
        ds.m_visualizeWhitespace = checked;
        d->editor->setDisplaySettings(ds);
    }
}

void CodeEditorManager::onTabActionTrigger(){
    auto sender = this->sender();
    if(sender==d->saveAction){
        d->current->save(false);
    }else if(sender==d->closeAction){
        auto container = d->current->container();
        container->closePane(d->current);
    }else if(sender==d->closeOtherAction){
        auto container = d->current->container();
        int index = container->indexOf(d->current);
        int count  = container->paneCount();
        int pos = 0;
        for(int i=0;i<count;i++){
            if(i==index){
                pos += 1;
            }else{
                if(!container->closePane(pos)){
                    pos += 1;
                }
            }
        }
    }else if(sender==d->closeAllAction){
        auto container = d->current->container();
        int count  = container->paneCount();
        int pos = 0;
        for(int i=0;i<count;i++){
            if(!container->closePane(pos)){
                pos += 1;
            }
        }
    }else if(sender==d->copyPathAction){
        QApplication::clipboard()->setText(d->current->path());
    }else if(sender==d->openFolderAction){
        QDesktopServices::openUrl(QFileInfo(d->current->path()).absoluteDir().absolutePath());
    }else if(sender==d->floatTabAction){
        auto container = d->current->container();
        int index = container->indexOf(d->current);
        if(index>=0)
            container->floatPane(index);
    }
}


void CodeEditorManager::editorContextMenu(CodeEditorView* editor,QMenu* contextMenu){
    if(d->undoAction==nullptr){
        d->undoAction = new QAction(QIcon(":/Resource/icons/Undo_16x.svg"),tr("Undo"),this);
        d->redoAction = new QAction(QIcon(":/Resource/icons/Redo_16x.svg"),tr("Redo"),this);
        d->copyAction = new QAction(QIcon(":/Resource/icons/Copy_16x.svg"),tr("Copy"),this);
        d->copyHtmlAction = new QAction(QIcon(":/Resource/icons/CopyLongTextToClipboard_16x.svg"),tr("Copy Html"),this);
        d->cutAction = new QAction(QIcon(":/Resource/icons/Cut_16x.svg"),tr("Cut"),this);
        d->pasteAction = new QAction(QIcon(":/Resource/icons/Paste_16x.svg"),tr("Paste"),this);
        d->selectAllAction = new QAction(QIcon(":/Resource/icons/SelectAll_16x.svg"),tr("Select All"),this);
        d->textWrappingAction = new QAction(QIcon(":/Resource/icons/WordWrap_16x.svg"),tr("Enable Text &Wrapping"),this);
        d->visualizeWhitespaceAction = new QAction(tr("Visualize Whitespace"),this);
        d->indentAction = new QAction(QIcon(":/Resource/icons/TextIndent_16x.svg"),tr("Indent Selection"),this);
        d->autoFormatAction = new QAction(QIcon(":/Resource/icons/FormatSelection_16x.svg"),tr("Auto Format"),this);
        d->addOrRemoveCommentAction = new QAction(QIcon(":/Resource/icons/CPPCommentCode_16x.svg"),tr("Add/Remove Comment"),this);

        d->visualizeWhitespaceAction->setCheckable(true);
        d->textWrappingAction->setCheckable(true);


        d->undoAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Z));
        d->redoAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Y));
        d->copyAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C));
        d->cutAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_X));
        d->pasteAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_V));
        d->selectAllAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_A));
        d->autoFormatAction->setShortcut(QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_F));

        connect(d->undoAction,&QAction::triggered,this,&CodeEditorManager::onEditorActionTrigger);
        connect(d->redoAction,&QAction::triggered,this,&CodeEditorManager::onEditorActionTrigger);
        connect(d->copyAction,&QAction::triggered,this,&CodeEditorManager::onEditorActionTrigger);
        //connect(d->copyHtmlAction,&QAction::triggered,this,&CodeEditorManager::onEditorActionTrigger);
        connect(d->cutAction,&QAction::triggered,this,&CodeEditorManager::onEditorActionTrigger);
        connect(d->pasteAction,&QAction::triggered,this,&CodeEditorManager::onEditorActionTrigger);
        connect(d->selectAllAction,&QAction::triggered,this,&CodeEditorManager::onEditorActionTrigger);
        connect(d->textWrappingAction,&QAction::triggered,this,&CodeEditorManager::onEditorActionTrigger);
        connect(d->visualizeWhitespaceAction,&QAction::triggered,this,&CodeEditorManager::onEditorActionTrigger);
        connect(d->indentAction,&QAction::triggered,this,&CodeEditorManager::onEditorActionTrigger);
        connect(d->autoFormatAction,&QAction::triggered,this,&CodeEditorManager::onEditorActionTrigger);
        connect(d->addOrRemoveCommentAction,&QAction::triggered,this,&CodeEditorManager::onEditorActionTrigger);
    }

    auto ds = editor->displaySettings();
    d->textWrappingAction->setChecked(ds.m_textWrapping);
    d->visualizeWhitespaceAction->setChecked(ds.m_visualizeWhitespace);

    d->pasteAction->setEnabled(editor->canPaste());
    //d->undoAction->setEnabled(editor->isUndoRedoEnabled());
    //d->redoAction->setEnabled(editor->isUndoRedoEnabled());


    d->editor = editor;
    contextMenu->addAction(d->textWrappingAction);
    contextMenu->addAction(d->visualizeWhitespaceAction);
    contextMenu->addSeparator();
    contextMenu->addAction(d->undoAction);
    contextMenu->addAction(d->redoAction);
    contextMenu->addSeparator();
    contextMenu->addAction(d->copyAction);
    //contextMenu->addAction(d->copyHtmlAction);
    contextMenu->addAction(d->cutAction);
    contextMenu->addAction(d->pasteAction);
    contextMenu->addSeparator();
    contextMenu->addAction(d->selectAllAction);
    contextMenu->addSeparator();
    contextMenu->addAction(d->indentAction);
    contextMenu->addAction(d->autoFormatAction);
    contextMenu->addSeparator();
    contextMenu->addAction(d->addOrRemoveCommentAction);
}


void CodeEditorManager::tabContextMenu(CodeEditorPane* pane,QMenu* contextMenu){
    if(d->saveAction==nullptr){
        d->saveAction = new QAction(QIcon(":/Resource/icons/Save_16x.svg"),tr("Save(&S)"),this);
        d->closeAction = new QAction(tr("Close(&C)"),this);
        d->closeOtherAction = new QAction(tr("Close Other(&A)"),this);
        d->closeAllAction = new QAction(tr("Close All(&L)"),this);
        d->copyPathAction = new QAction(QIcon(":/Resource/icons/Copy_16x.svg"),tr("Copy Path(&U)"),this);
        d->openFolderAction = new QAction(tr("Open Folder(&O)"),this);
        d->floatTabAction = new QAction(tr("Float Tab(&F)"),this);
        d->saveAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
        d->closeAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F4));
        connect(d->saveAction,&QAction::triggered,this,&CodeEditorManager::onTabActionTrigger);
        connect(d->closeAction,&QAction::triggered,this,&CodeEditorManager::onTabActionTrigger);
        connect(d->closeOtherAction,&QAction::triggered,this,&CodeEditorManager::onTabActionTrigger);
        connect(d->closeAllAction,&QAction::triggered,this,&CodeEditorManager::onTabActionTrigger);
        connect(d->copyPathAction,&QAction::triggered,this,&CodeEditorManager::onTabActionTrigger);
        connect(d->openFolderAction,&QAction::triggered,this,&CodeEditorManager::onTabActionTrigger);
        connect(d->floatTabAction,&QAction::triggered,this,&CodeEditorManager::onTabActionTrigger);
    }
    d->current = pane;
    contextMenu->addAction(d->saveAction);
    contextMenu->addAction(d->closeAction);
    contextMenu->addAction(d->closeOtherAction);
    contextMenu->addAction(d->closeAllAction);
    contextMenu->addSeparator();
    contextMenu->addAction(d->copyPathAction);
    contextMenu->addAction(d->openFolderAction);
    contextMenu->addSeparator();
    contextMenu->addAction(d->floatTabAction);
}
}
