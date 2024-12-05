#ifndef SITE_QUICK_MANAGER_DIALOG_H
#define SITE_QUICK_MANAGER_DIALOG_H

#include <w_dialog.h>

namespace Ui {
class SiteQuickManagerDialog;
}

namespace ady{
class SiteQuickManagerDialogPrivate;
class SiteQuickManagerDialog : public wDialog
{
    Q_OBJECT

public:

    ~SiteQuickManagerDialog();

    void initData();

    static SiteQuickManagerDialog* open(QWidget *parent);

public slots:
    void onNewSite();
    void onSiteChanged(int i);
    void onTypeChanged(int i);
    void onAdjustHeight(int i);
    void onDeleteSite(int i);
    void onSave();
    void onDel();
    void onConnect();

private:
    explicit SiteQuickManagerDialog(QWidget *parent = nullptr);
    void initSiteList();

private:
    Ui::SiteQuickManagerDialog *ui;
    SiteQuickManagerDialogPrivate* d;
    static SiteQuickManagerDialog* instance;
};

}
#endif // SITE_QUICK_MANAGER_DIALOG_H
