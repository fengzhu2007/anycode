#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "global.h"
#include <QMainWindow>
#include <QMap>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
namespace ady {
    class LocalDirPanel;
    class Panel;
    class StatusBarView;
    class LogViewer;
    class ANYENGINE_EXPORT MainWindow : public QMainWindow
    {
        Q_OBJECT

    public:
        enum RemoveTaskFlag {
            RM_All=0,
            RM_Selected,
            RM_All_Failed
        };

        enum RetryTaskFlag {
            RT_All_Failed,
            RT_Selected
        };

        MainWindow(QWidget *parent = nullptr);
        ~MainWindow();
        void initView();
        void initData();
        Panel* panel(long long id);
        QMap<long long,Panel*> panels();
        bool cvsMode();


    private:
        void createStatusBar();


    public slots:
        void onNew();
        void onOpen();
        void onClose();
        void onProjectOpened(long long id);
        void onProjectClose();
        void onTabClosed(int index);
        void onSwitchCVS(bool checked);

        void onTaskExpandAll();

        void onStart();
        void onStop();

        void onContextMenu(const QPoint& pos);

        void onActTaskRemove();
        void onActTaskRetry();

        //void onNetworkUpdate();
        //void onDnsLookedup();
        void onNetworkStateChanged(bool isOnline);
        void onStatusMessage(const QString& name);

        void onDonation();
        void onAbout();
        void onWebsite();
        void onExport();
        void onImport();
        void onAddonManage();


        void onViewLog();




    private:
        Ui::MainWindow *ui;
        LocalDirPanel *localDirPanel;
        QMap<long long,Panel*> m_panels;
        long long m_projectId;
        StatusBarView* statusBar;
        LogViewer* m_logViewer;
    };
}

#endif // MAINWINDOW_H
