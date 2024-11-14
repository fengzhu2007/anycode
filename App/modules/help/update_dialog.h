#ifndef UPDATE_DIALOG_H
#define UPDATE_DIALOG_H

#include <w_dialog.h>

namespace Ui {
class UpdateDialog;
}

namespace ady{
class UpdateDialog : public wDialog
{
    Q_OBJECT

public:
    explicit UpdateDialog(QWidget *parent = nullptr);
    ~UpdateDialog();

    static UpdateDialog* open(QWidget* parent);

protected:
    void closeEvent(QCloseEvent* e) override;



private:
    Ui::UpdateDialog *ui;
    static UpdateDialog* instance;
};
}
#endif // UPDATE_DIALOG_H
