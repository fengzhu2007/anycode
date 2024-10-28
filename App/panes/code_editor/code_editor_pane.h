#ifndef CODEEDITORPANE_H
#define CODEEDITORPANE_H
#include "global.h"
#include "docking_pane.h"
#include "code_editor_view.h"
//#include "core/event_bus/subscriber.h"

namespace ady{
class DockingPaneManager;
class CodeEditor;
class LineNumberArea;
class CodeEditorPanePrivate;
class ANYENGINE_EXPORT CodeEditorPane : public DockingPane
{
    Q_OBJECT
public:
    enum FileState{
        None=0,
        Deleted=1,
        Changed=2
    };

    explicit CodeEditorPane(QWidget *parent = nullptr);
    virtual ~CodeEditorPane();
    virtual QString id() override;
    virtual QString group() override;
    virtual QString description() override;
    virtual void activation() override;
    virtual void save(bool rename) override;
    virtual void contextMenu(const QPoint& pos) override;
    virtual QJsonObject toJson() override;
    //virtual bool onReceive(Event* e) override;//event bus receive callback
    void rename(const QString& name);
    bool readFile(const QString& path);
    bool writeFile(const QString& path);
    QString path();
    bool isModified() const;
    //CodeEditor* editor();
    CodeEditorView* editor();

    int fileState();
    void setFileState(int state);
    void invokeFileState();

    bool isModification();

    void reload();



    static CodeEditorPane* make(DockingPaneManager* dockingManager,const QJsonObject& data);

public slots:
    void onModificationChanged(bool changed);

protected:
    virtual void showEvent(QShowEvent* e) override;


public:
    static const QString PANE_ID;
    static const QString PANE_GROUP;


private:
    CodeEditorPanePrivate* d;
    static int SN;

};


}
#endif // CODEEDITOR_H
