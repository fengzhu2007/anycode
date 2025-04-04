#ifndef CHMOD_DIALOG_H
#define CHMOD_DIALOG_H
#include <QDialog>

namespace Ui {
class ChmodDialogUI;
}

namespace ady {
    class ChmodDialog : public QDialog
    {
        Q_OBJECT
    public:
        ChmodDialog(int mode,QWidget* parent=nullptr);
        ~ChmodDialog();
        int mode();
        bool applyChildren();
        void setFileInfo(QString file,int count=1);
        static int toFormat(QString mode);

    public slots:
        void onOK();
    private:
        Ui::ChmodDialogUI* ui;

    };
}
#endif // CHMOD_DIALOG_H
