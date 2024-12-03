#ifndef IDEWINDOW_H
#define IDEWINDOW_H
#include "global.h"
#include "w_main_window.h"
#include "core/event_bus/subscriber.h"
//#include "storage/project_storage.h"

namespace Ui {
class IDEWindow;
}

namespace ady{
    class CodeEditorPane;
    class DockingPaneLayoutItemInfo;
    class DockingPaneManager;
    class DockingPaneContainer;
    class DockingPane;
    class IDEWindowPrivate;
    class ProjectRecord;
    class ANYENGINE_EXPORT IDEWindow : public wMainWindow , public Subscriber
    {
        Q_OBJECT

    public:
        explicit IDEWindow(QWidget *parent = nullptr);
        ~IDEWindow();
        virtual bool onReceive(Event* e) override;//event bus receive callback
        void initView();
        void runExe(const QString& executable,const QStringList& arguments);
        void openUrl(const QString& url);

        void restart();

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
        void onRecentMenuShow();



    protected:
        //virtual void closeEvent(QCloseEvent *event) override;
        virtual void showEvent(QShowEvent* e) override;

    private:
        void onOpenFindAndReplace(int mode,const QString& text,const QString& scope);
        void boot();
        void delayBoot();//delay boot
        void shutdown();
        void forTest();

    private:
        CodeEditorPane* currentEditorPane();
        void restoreFromSettings();
        void restoreDockpanes();
        void restoreProjects();

        void openProject(ProjectRecord& proj);

    private:
        Ui::IDEWindow *ui;
        DockingPaneManager* m_dockingPaneManager;
        IDEWindowPrivate* d;
    };
}




#endif // IDEWINDOW_H
