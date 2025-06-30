#ifndef FRAMES_MERGE_DIALOG_H
#define FRAMES_MERGE_DIALOG_H

#include <w_dialog.h>

namespace Ui {
class FramesMergeDialog;
}

namespace ady{
class FramesModel;
class FramesMergeDialogPrivate;
class FramesMergeDialog : public wDialog
{
    Q_OBJECT

public:
    explicit FramesMergeDialog(QWidget *parent = nullptr);
    ~FramesMergeDialog();
    void saveTo(const QString& outputDir);
    void setModel(FramesModel* model);
    void merge();
    static FramesMergeDialog* open(QWidget* parent);


public slots:
    void onSave();
    void onColumnsChanged(int n);


private:
    Ui::FramesMergeDialog *ui;
    FramesMergeDialogPrivate* d;
    static FramesMergeDialog* instance;
};
}

#endif // FRAMES_MERGE_DIALOG_H
