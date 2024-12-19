#ifndef IMPORT_EXPORT_DIALOG_H
#define IMPORT_EXPORT_DIALOG_H

#include <w_dialog.h>

namespace Ui {
class ImportExportDialog;
}

namespace ady{
class ImportExportDialogPrivate;
class ImportExportDialog : public wDialog
{
    Q_OBJECT

public:
    enum Step{
        Select=0,
        Export,
        Export_To,
        Import_Select,
        Import
    };
    static ImportExportDialog* getInstance();
    static ImportExportDialog* open(QWidget* parent);

    ~ImportExportDialog();
    void initView();
    void startup();

public slots:
    void onClicked();
    void onCurrentChanged(int i);

private:
    explicit ImportExportDialog(QWidget *parent = nullptr);

private:
    Ui::ImportExportDialog *ui;
    ImportExportDialogPrivate* d;
    static ImportExportDialog* instance;
};
}

#endif // IMPORT_EXPORT_DIALOG_H
