#include "code_editor_manager.h"
#include "code_editor_pane.h"
#include "code_editor.h"
#include "docking_pane_manager.h"
#include "docking_pane_container.h"
#include "core/event_bus/event.h"
#include "components/message_dialog.h"
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>
#include <QThread>
#include <QFileSystemWatcher>
#include <QDebug>
namespace ady{

const QString CodeEditorManager::M_WILL_RENAME_FILE = "WillRenameFile";
const QString CodeEditorManager::M_WILL_RENAME_FOLDER = "WillRenameFolder";
const QString CodeEditorManager::M_RENAMED_FILE = "RenamedFile";
const QString CodeEditorManager::M_RENAMED_FOLDER = "RenamedFolder";
const QString CodeEditorManager::M_RELOAD_FILE = "ReloadFile";
const QString CodeEditorManager::M_DELETE_FILE = "DeleteFile";
const QString CodeEditorManager::M_DELETE_FOLDER = "DeleteFolder";


CodeEditorManager* CodeEditorManager::instance = nullptr;

class CodeEditorManagerPrivate{
public:
    QList<CodeEditorPane*> list;
    QMutex mutex;
    DockingPaneManager* docking_manager;
    QFileSystemWatcher *watcher;
};

CodeEditorManager::CodeEditorManager(DockingPaneManager* docking_manager)
{
    Subscriber::reg();
    this->regMessageIds({M_WILL_RENAME_FILE,M_RENAMED_FILE,M_WILL_RENAME_FOLDER,M_RENAMED_FOLDER,M_RELOAD_FILE,M_DELETE_FILE,M_DELETE_FOLDER});
    d = new CodeEditorManagerPrivate;
    d->docking_manager = docking_manager;
    d->watcher = new QFileSystemWatcher(this);
    connect(d->watcher,&QFileSystemWatcher::fileChanged,this,&CodeEditorManager::onFileChanged);
}

CodeEditorManager::~CodeEditorManager(){
    Subscriber::unReg();
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

CodeEditorPane* CodeEditorManager::open(DockingPaneManager* dockingManager,const QString& path){
    auto pane = this->get(path);
    if(pane==nullptr){
        pane = new CodeEditorPane();
        if(path.isEmpty()){
            pane->setWindowTitle(QObject::tr("New File"));
        }else{
            d->watcher->addPath(path);
            QFileInfo fi(path);
            pane->setWindowTitle(fi.fileName());
            pane->readFile(path);
        }
        dockingManager->createPane(pane,DockingPaneManager::Center,true);
    }else{
        //set pane to current
        pane->activeToCurrent();
    }
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
    if(pane!=nullptr){
        int i = 0;
        while(pane->editor()->isShowed()==false && i++<100){
            QThread::msleep(2);
        }
        QMetaObject::invokeMethod(pane->editor(), "fillPlainText", Qt::AutoConnection,Q_ARG(QString, text));
    }
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
    if(id==M_WILL_RENAME_FILE){
        auto data = static_cast<QString*>(e->data());
        this->removeWatchFile(*data);
        e->ignore();
        return true;
    }else if(id==M_RENAMED_FILE){
        auto pair = static_cast<QPair<QString,QString>*>(e->data());
        if(this->rename(pair->first,pair->second)){
            this->appendWatchFile(pair->second);
        }
        e->ignore();
        return true;
    }else if(id==M_WILL_RENAME_FOLDER){
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
    }else if(id==M_RENAMED_FOLDER){
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
    }else if(id==M_RELOAD_FILE){
        auto data = static_cast<QString*>(e->data());
        auto pane = this->get(*data);
        if(pane!=nullptr){
            pane->readFile(*data);
        }
    }else if(id==M_DELETE_FILE){
        auto data = static_cast<QString*>(e->data());
        auto pane = this->get(*data);
        if(pane!=nullptr){
            auto container = pane->container();
            if(container!=nullptr){
                container->closePane(pane,true);
            }
        }
    }else if(id==M_DELETE_FOLDER){
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
        QFileInfo fi(path);
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
        }
    }


}

}
