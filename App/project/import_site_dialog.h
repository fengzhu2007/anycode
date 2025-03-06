#ifndef IMPORT_SITE_DIALOG_H
#define IMPORT_SITE_DIALOG_H

#include <w_dialog.h>

namespace Ui {
class ImportSiteDialog;
}

namespace ady{
class ImportSiteDialog : public wDialog
{
    Q_OBJECT

public:
    explicit ImportSiteDialog(QWidget *parent = nullptr);
    ~ImportSiteDialog();

    static ImportSiteDialog* open(QWidget* parent);


public slots:
    void onSelected(const QModelIndex& index);

signals:
    void selected(long long id);

private:
    Ui::ImportSiteDialog *ui;
};
}

#endif // IMPORT_SITE_DIALOG_H
