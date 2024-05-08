#ifndef DONATIONDIALOG_H
#define DONATIONDIALOG_H
#include <QDialog>

namespace Ui {
    class DonationDialog;
}


namespace ady {
    class DonationDialog : public QDialog
    {
        Q_OBJECT

    public:
        DonationDialog(QWidget* parent);
        ~DonationDialog();

    private:
        Ui::DonationDialog *ui;

    };
}
#endif // DONATIONDIALOG_H
