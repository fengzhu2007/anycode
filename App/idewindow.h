#ifndef IDEWINDOW_H
#define IDEWINDOW_H
#include "global.h"
#include "w_main_window.h"

namespace Ui {
class IDEWindow;
}

namespace ady{
    class DockingPaneManager;
    class DockingPane;
    class ANYENGINE_EXPORT IDEWindow : public wMainWindow
    {
        Q_OBJECT

    public:
        explicit IDEWindow(QWidget *parent = nullptr);
        ~IDEWindow();

    public slots:
        void onDump();
        void onPaneClosed(QString&id,QString&group,bool isClient);
        void onBeforePaneClose(DockingPane* pane,bool isClient);

        void onActionTriggered();

    private:
        Ui::IDEWindow *ui;
        DockingPaneManager* m_dockingPaneManager;
    };
}




#endif // IDEWINDOW_H
