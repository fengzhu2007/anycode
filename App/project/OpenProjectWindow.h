#ifndef OPENPROJECTWINDOW_H
#define OPENPROJECTWINDOW_H
#include "global.h"
#include <QDialog>
namespace Ui {
class OpenProjectWindow;
}
namespace ady {
    class ANYENGINE_EXPORT OpenProjectWindow : public QDialog
    {
        Q_OBJECT
    public:
        OpenProjectWindow(QWidget* parent);
        void initData();

    public slots:
        void onSelected();
    signals:
        void selectionChanged(long long id);

    private:
        Ui::OpenProjectWindow *ui;

    };
}
#endif // OPENPROJECTWINDOW_H
