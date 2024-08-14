#ifndef REPLACE_ALL_PROGRESS_DIALOG_H
#define REPLACE_ALL_PROGRESS_DIALOG_H
#include "w_dialog.h"
#include <QDialog>

namespace Ui {
class ReplaceAllProgressDialog;
}

namespace ady{
class ReplaceAllProgressDialog : public wDialog
{
    Q_OBJECT

public:
    explicit ReplaceAllProgressDialog(QWidget *parent = nullptr);
    ~ReplaceAllProgressDialog();
    static ReplaceAllProgressDialog* open(QWidget* parent);
    static ReplaceAllProgressDialog* getInstance(){
        return instance;
    }

private:
    Ui::ReplaceAllProgressDialog *ui;
    static ReplaceAllProgressDialog* instance;
};
}
#endif // REPLACE_ALL_PROGRESS_DIALOG_H
