#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H
#include <w_dialog.h>
#include <QDialog>

namespace Ui {
    class AboutDialog;
}


namespace ady {
    class AboutDialog : public wDialog
    {
        Q_OBJECT

    public:
        explicit AboutDialog(QWidget* parent);
        ~AboutDialog();
        static AboutDialog* open(QWidget* parent);

    private:
        Ui::AboutDialog *ui;
        static AboutDialog* instance;

    };
}
#endif // ABOUTDIALOG_H
