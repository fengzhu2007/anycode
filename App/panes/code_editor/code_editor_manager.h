#ifndef CODEEDITORMANAGER_H
#define CODEEDITORMANAGER_H

#include "global.h"
#include <QObject>
#include "core/event_bus/subscriber.h"
namespace ady{
class DockingPaneManager;
class CodeEditorPane;
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
    CodeEditorPane* open(DockingPaneManager* dockingManager,const QString& path);
    CodeEditorPane* open(const QString& path);
    void append(CodeEditorPane* pane);
    void remove(CodeEditorPane* pane);
    bool readFileLines(const QString&path);
    void appendToEditor(const QString& path,const QString& text);
    bool appendWatchFile(const QString& path);
    bool removeWatchFile(const QString& path);
    bool rename(const QString& from,const QString& to);

    virtual bool onReceive(Event*) override;

public slots:
    void onFileChanged(const QString &path);

private:
    CodeEditorManager(DockingPaneManager* docking_manager);

public:
    static const QString M_WILL_RENAME_FILE;
    static const QString M_WILL_RENAME_FOLDER ;
    static const QString M_RENAMED_FILE;
    static const QString M_RENAMED_FOLDER;
    static const QString M_RELOAD_FILE;
    static const QString M_DELETE_FILE;
    static const QString M_DELETE_FOLDER;

private:
    static CodeEditorManager* instance;
    CodeEditorManagerPrivate* d;
};

}

#endif // CODEEDITORMANAGER_H
