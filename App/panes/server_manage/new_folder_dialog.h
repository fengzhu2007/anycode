#ifndef NEWFOLDERDIALOG_H
#define NEWFOLDERDIALOG_H
#include "global.h"
#include <QDialog>
#include "w_dialog.h"

namespace Ui {
class NewFolderDialog;
}

namespace ady{
class NewFolderDialogPrivate;
    class ANYENGINE_EXPORT NewFolderDialog : public wDialog
    {
        Q_OBJECT
    public:
        explicit NewFolderDialog(long long id,const QString& path,QWidget* parent=nullptr);
        ~NewFolderDialog();

        static NewFolderDialog* open(long long id,const QString& path,QWidget* parent);

    public slots:
        void onOk();

    signals:
        void submit(long long id,const QString& path,const QString name);

    private:
        Ui::NewFolderDialog *ui;
        NewFolderDialogPrivate *d;
    };
}

#endif // NEWFOLDERDIALOG_H
