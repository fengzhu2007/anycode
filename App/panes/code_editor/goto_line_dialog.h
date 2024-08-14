#ifndef GOTO_LINE_DIALOG_H
#define GOTO_LINE_DIALOG_H

#include "w_dialog.h"

namespace Ui {
class GotoLineDialog;
}

namespace ady{
class GotoLineDialogPrivate;
class GotoLineDialog : public wDialog
{
    Q_OBJECT

public:
    static GotoLineDialog* getInstance();
    static GotoLineDialog* open(QWidget* parent,int current=-1,int max=-1);
    ~GotoLineDialog();
    void setLineRange(int max);
    void setCurrentLineNo(int no);

public slots:
    void onGoto();

private:
    explicit GotoLineDialog(QWidget *parent = nullptr);

private:
    Ui::GotoLineDialog *ui;
    static GotoLineDialog* instance;
    GotoLineDialogPrivate* d;
};
}

#endif // GOTO_LINE_DIALOG_H
