#ifndef IDEWINDOW_H
#define IDEWINDOW_H
#include "global.h"
#include "w_main_window.h"
#include "core/event_bus/subscriber.h"

namespace Ui {
class IDEWindow;
}

namespace ady{
    class CodeEditorPane;
    class DockingPaneContainer;
    class DockingPaneManager;
    class DockingPane;
    class ANYENGINE_EXPORT IDEWindow : public wMainWindow , public Subscriber
    {
        Q_OBJECT

    public:
        explicit IDEWindow(QWidget *parent = nullptr);
        ~IDEWindow();
        virtual bool onReceive(Event* e) override;//event bus receive callback

    public slots:
        void onDump();
        void onPaneClosed(QString&id,QString&group,bool isClient);
        void onBeforePaneClose(DockingPane* pane,bool isClient);
        void onActionTriggered();
        void onOpenFile(const QString& path);
        void onOpenFolder(const QString& path);
        void onSearch(const QString& text,int flags,bool hightlight=true);
        void onSearchAll(const QString& text,const QString& scope,int flags,const QString& filter,const QString& exclusion);
        void onReplaceAll(const QString& before,const QString& after,const QString& scope,int flags,const QString& filter,const QString& exclusion);
        void onReplace(const QString& before,const QString& after,int flags,bool hightlight=true);
        void onSearchCancel();



    protected:
        //virtual void closeEvent(QCloseEvent *event) override;

    private:
        void onOpenFindAndReplace(int mode,const QString& text,const QString& scope);
        void forTest();

    private:
        CodeEditorPane* currentEditorPane();
        void restoreFromSettings();
        void restoreDockpanes();
        void restoreDockContainers(QJsonArray& list,int orientation,DockingPaneContainer* relation);
        void restoreProjects();

    private:
        Ui::IDEWindow *ui;
        DockingPaneManager* m_dockingPaneManager;
    };
}




#endif // IDEWINDOW_H
