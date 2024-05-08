#ifndef ADDONMANAGEWINDOW_H
#define ADDONMANAGEWINDOW_H
#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
    class AddonManageWindow;
}
QT_END_NAMESPACE

namespace ady {

    class AddonManageWindow : public QDialog {
        Q_OBJECT
    private:
        AddonManageWindow(QWidget* parent);
        ~AddonManageWindow();

    public:
        static AddonManageWindow* getInstance(QWidget*parent);



    private:
        Ui::AddonManageWindow* ui;
        static AddonManageWindow* instance;
    };

};
#endif // ADDONMANAGEWINDOW_H
