#ifndef DOCKWINDOW_H
#define DOCKWINDOW_H
#include "global.h"
#include <QMainWindow>
QT_BEGIN_NAMESPACE
namespace Ui {
class DockWindow;
}
QT_END_NAMESPACE



namespace ady{
    class DockingPaneManager;

    class ANYENGINE_EXPORT DockWindow : public QMainWindow
    {
        Q_OBJECT

    public:
        explicit DockWindow(QWidget *parent = nullptr);
        ~DockWindow();

    public slots:
        void onDump();
        void onShow();

    private:
        Ui::DockWindow *ui;
        DockingPaneManager* m_dockingPaneManager;
    };
}



#endif // DOCKWINDOW_H
