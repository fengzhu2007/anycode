#ifndef PERMISSIONDIALOG_H
#define PERMISSIONDIALOG_H

#include "global.h"
#include "w_dialog.h"

namespace Ui {
class PermissionDialog;
}

namespace ady{

class PermissionDialogPrivate;


class ANYENGINE_EXPORT PermissionDialog : public wDialog
{
    Q_OBJECT
public:
    explicit PermissionDialog(int mode,QWidget* parent=nullptr);
    ~PermissionDialog();
    int mode();
    bool applyChildren();
    void setFileInfo(const QString& file,int count=1);
    static int format(const QString& permission);

public slots:
    void onOK();

private:
    //PermissionDialogPrivate* d;
    Ui::PermissionDialog* ui;
};

}

#endif // PERMISSIONDIALOG_H
