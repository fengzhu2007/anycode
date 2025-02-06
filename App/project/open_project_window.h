#ifndef OPENPROJECTWINDOW_H
#define OPENPROJECTWINDOW_H
#include "global.h"
#include <QDialog>
#include "w_dialog.h"
namespace Ui {
class OpenProjectWindow;
}
namespace ady {
    class ANYENGINE_EXPORT OpenProjectWindow : public wDialog
    {
        Q_OBJECT
    public:
        ~OpenProjectWindow();
        void initData();
        static OpenProjectWindow* open(QWidget* parent);

    protected:
        void showEvent(QShowEvent* e) override;

    private:
        explicit OpenProjectWindow(QWidget* parent);

    public slots:
        void onSelected();
    signals:
        void selectionChanged(long long id);

    private:
        Ui::OpenProjectWindow *ui;
        static OpenProjectWindow* instance;

    };
}
#endif // OPENPROJECTWINDOW_H
