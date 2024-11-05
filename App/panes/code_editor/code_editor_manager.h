#ifndef CODEEDITORMANAGER_H
#define CODEEDITORMANAGER_H

#include "global.h"
#include "core/event_bus/subscriber.h"
//#include "textdocument.h"
#include <QTextDocument>
#include <QMenu>
namespace ady{
class DockingPaneManager;
class CodeEditorPane;
class CodeEditorView;
class CodeEditorManagerPrivate;
class ANYENGINE_EXPORT CodeEditorManager : public QObject,public Subscriber
{
    Q_OBJECT
public:
    ~CodeEditorManager();
    static CodeEditorManager* getInstance();
    static CodeEditorManager* init(DockingPaneManager* docking_manager=nullptr);
    static void destory();
    CodeEditorPane* get(const QString& path);
    QList<CodeEditorPane*> getAll(const QString& prefix);
    CodeEditorPane* current();
    void setCurrent(CodeEditorPane* pane);
    QTextDocument* currentDoc();
    QMap<QString,QTextDocument*> docData();

    CodeEditorPane* open(DockingPaneManager* dockingManager,const QString& path,int line=0,int column=0);
    CodeEditorPane* open(const QString& path);




    void append(CodeEditorPane* pane);
    void remove(CodeEditorPane* pane);
    bool readFileLines(const QString&path);
    void appendToEditor(const QString& path,const QString& text);
    bool appendWatchFile(const QString& path);
    bool removeWatchFile(const QString& path);
    bool rename(const QString& from,const QString& to);

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
