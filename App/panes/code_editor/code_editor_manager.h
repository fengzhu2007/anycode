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
using EditorFactoryFunc = std::function<Editor*(const QJsonObject&)>;

class DockingPaneManager;
class CodeEditorPane;
class CodeEditorView;
class Editor;
class LanguageSettings;
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

    //CodeEditorPane* open(DockingPaneManager* dockingManager,const QString& path,int line=0,int column=0);
    //CodeEditorPane* open(const QString& path);

    Editor* open(const QString& path,const QString& localPath={},const QString& extension={});

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

    template <typename T>
    void registerEditor(const QString& suffix){
        auto docking = this->dockingManager();
        m_factoryList[suffix] = [docking](const QJsonObject& data){
            return T::make(docking,data);
        };
    }

    void append(Editor* pane);
    void remove(Editor* pane);
    bool readFileLines(const QString&path);
    void appendToEditor(const QString& path,const QString& text);
    bool appendWatchFile(const QString& path);
    bool removeWatchFile(const QString& path);
    bool rename(const QString& from,const QString& to);
    void autoSave();

    virtual bool onReceive(Event*) override;

    void editorContextMenu(CodeEditorView* editor,QMenu* contextMenu);
    void tabContextMenu(Editor* pane,QMenu* contextMenu);
    DockingPaneManager* dockingManager();

public slots:
    void onFileChanged(const QString &path);
    void onEditorActionTrigger(bool checked=false);
    void onTabActionTrigger();
    void onLanguageSettingChanged(const LanguageSettings& setting);

private:
    CodeEditorManager(DockingPaneManager* docking_manager);
    void initEditors();


private:
    static CodeEditorManager* instance;
    CodeEditorManagerPrivate* d;
    std::map<QString,EditorFactoryFunc> m_factoryList;
};

}

#endif // CODEEDITORMANAGER_H
