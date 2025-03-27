#ifndef SQLITE_CONNECT_DIALOG_H
#define SQLITE_CONNECT_DIALOG_H

#include <w_dialog.h>
#include "global.h"
#include <QAbstractButton>

namespace Ui {
class SQLiteConnectDialog;
}

namespace ady{
class DBRecord;
class SQLiteConnectDialogPrivate;
class ANYENGINE_EXPORT SQLiteConnectDialog : public wDialog
{
    Q_OBJECT

public:
    explicit SQLiteConnectDialog(const DBRecord& data,QWidget *parent = nullptr);
    ~SQLiteConnectDialog();

    void initView();

private:
    bool validate();

public slots:
    void onTypeChanged(QAbstractButton *button, bool checked);
    void onSave();
    void onTest();

private:
    Ui::SQLiteConnectDialog *ui;
    SQLiteConnectDialogPrivate * d;
};

}

#endif // SQLITE_CONNECT_DIALOG_H
