#ifndef OPTIONS_DIALOG_H
#define OPTIONS_DIALOG_H

#include <w_dialog.h>

namespace Ui {
class OptionsDialog;
}
namespace ady{
class OptionsDialogPrivate;
class OptionsDialog : public wDialog
{
    Q_OBJECT
public:
    ~OptionsDialog();
    void initView();

    static OptionsDialog* open(QWidget* parent);

    void setCurrentIndex(int row);

    void notifyChanged(const QString& name,const QVariant& value);

protected:
    void keyPressEvent(QKeyEvent *e);

public slots:
    void onActivate(const QModelIndex& index);
    void onSave();
    void onFilter();

private:
    explicit OptionsDialog(QWidget* parent);


private:
    Ui::OptionsDialog* ui;
    OptionsDialogPrivate* d;
    static OptionsDialog* instance;
};
}
#endif // OPTIONS_DIALOG_H
