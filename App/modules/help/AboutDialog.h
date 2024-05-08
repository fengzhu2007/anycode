#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H
#include <QDialog>

namespace Ui {
    class AboutDialog;
}


namespace ady {
    class AboutDialog : public QDialog
    {
        Q_OBJECT

    public:
        AboutDialog(QWidget* parent);
        ~AboutDialog();

    private:
        Ui::AboutDialog *ui;

    };
}
#endif // ABOUTDIALOG_H
