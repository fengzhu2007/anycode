#ifndef MANAGERWINDOW_H
#define MANAGERWINDOW_H
#include "global.h"
#include <QDialog>
namespace Ui {
class ManagerWindow;
}
namespace ady {
    class ManagerTreeModel;
    class SplitterHandleWidget;
    class ANYENGINE_EXPORT ManagerWindow : public QDialog
    {
        Q_OBJECT
    public:
        ManagerWindow(QWidget* parent);
        void initData();
        void initView();
        inline ManagerTreeModel* getTreeModel(){return this->model;}



    public slots:
        void onProjectSave();
        void onSiteSave();
        void onSiteTest();
        void onTreeItemClicked(const QModelIndex& index);
        void onTriggered(QAction* a,bool checked);



    private:
        Ui::ManagerWindow *ui;
        ManagerTreeModel *model;
        SplitterHandleWidget *handle;
    };
}
#endif // MANAGERWINDOW_H
