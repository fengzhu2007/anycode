#ifndef FIND_REPLACE_DIALOG_H
#define FIND_REPLACE_DIALOG_H
#include "global.h"
#include <QDialog>
#include "w_dialog.h"
#include <QHideEvent>
namespace Ui {
class FindReplaceDialog;
}

namespace ady{
class FindReplaceDialogPrivate;
class ANYENGINE_EXPORT FindReplaceDialog : public wDialog
{
    Q_OBJECT

public:
    ~FindReplaceDialog();
    static FindReplaceDialog* getInstance();
    static FindReplaceDialog* open(QWidget* parent);

protected:
    virtual void hideEvent(QHideEvent *event) override;

signals:
    void search(const QString& text,int flags,bool hightlight=true);
    void cancelSearch();

private:
    explicit FindReplaceDialog(QWidget *parent = nullptr);

private:
    Ui::FindReplaceDialog *ui;
    FindReplaceDialogPrivate* d;
    static FindReplaceDialog* instance;
};
}
#endif // FIND_REPLACE_DIALOG_H
