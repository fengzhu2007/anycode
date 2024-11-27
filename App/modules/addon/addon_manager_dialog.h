#ifndef ADDON_MANAGER_DIALOG_H
#define ADDON_MANAGER_DIALOG_H

#include <w_dialog.h>

namespace Ui {
class AddonManagerDialog;
}
namespace ady{
class AddonManagerDialogPrivate;
class AddonManagerDialog : public wDialog
{
    Q_OBJECT

public:

    ~AddonManagerDialog();
    void initView();
    static AddonManagerDialog* open(QWidget* parent);

public slots:
    void onItemClicked(int index);

private:
    explicit AddonManagerDialog(QWidget *parent = nullptr);


private:
    Ui::AddonManagerDialog *ui;
    AddonManagerDialogPrivate* d;
    static AddonManagerDialog* instance;
};
}
#endif // ADDON_MANAGER_DIALOG_H
