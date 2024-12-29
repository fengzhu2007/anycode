#ifndef CODEEDITORMANAGER_H
#define CODEEDITORMANAGER_H

#include "global.h"
#include "core/event_bus/subscriber.h"
//#include "textdocument.h"
#include "panes/resource_manager/resource_manager_opened_model.h"
#include <docking_pane_manager.h>
#include <docking_pane.h>
#include "editor.h"
#include "storage/recent_storage.h"
#include <QTextDocument>
#include <QMenu>
namespace ady{
class DockingPaneManager;
class CodeEditorPane;
class CodeEditorView;
class Editor;
class CodeEditorManagerPrivate;
class ANYENGINE_EXPORT CodeEditorManager : public QObject,public Subscriber
{
    Q_OBJECT
public:
    ~CodeEditorManager();
    static CodeEditorManager* getInstance();
    static CodeEditorManager* init(DockingPaneManager* docking_manager=nullptr);
    static void destory();
    static QStringList openedFiles();
    Editor* get(const QString& path);
    QList<Editor*> getAll(const QString& prefix);
    Editor* current();
    void setCurrent(Editor* pane);
    QTextDocument* currentDoc();
    QMap<QString,QTextDocument*> docData();

    CodeEditorPane* open(DockingPaneManager* dockingManager,const QString& path,int line=0,int column=0);
    CodeEditorPane* open(const QString& path);

    template<class T>
    T* CodeEditorManager::open(DockingPaneManager* dockingManager,const QString& path){
        T* pane = dynamic_cast<T*>(this->get(path));
        if(pane==nullptr){
            pane = T::make(dockingManager);
            dockingManager->createPane(pane,DockingPaneManager::Center,true);
            //add to open
            ResourceManagerOpenedModel::getInstance()->appendItem(path);
        }else{
            //set pane to current
            pane->activeToCurrent();
        }
        if(path.isEmpty()==false){
            RecentStorage().add(RecentStorage::File,path);//add to recent
        }
        return pane;
    }

    void append(CodeEditorPane* pane);
    void remove(CodeEditorPane* pane);
    bool readFileLines(const QString&path);
    void appendToEditor(const QString& path,const QString& text);
    bool appendWatchFile(const QString& path);
    bool removeWatchFile(const QString& path);
    bool rename(const QString& from,const QString& to);
    void autoSave();

    virtual bool onReceive(Event*) override;

    void editorContextMenu(CodeEditorView* editor,QMenu* contextMenu);
    void tabContextMenu(CodeEditorPane* pane,QMenu* contextMenu);


public slots:
    void onFileChanged(const QString &path);
    void onEditorActionTrigger(bool checked=false);
    void onTabActionTrigger();

private:
    CodeEditorManager(DockingPaneManager* docking_manager);


private:
    static CodeEditorManager* instance;
    CodeEditorManagerPrivate* d;
};

}

#endif // CODEEDITORMANAGER_H
