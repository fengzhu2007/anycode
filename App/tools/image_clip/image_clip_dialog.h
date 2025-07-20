#ifndef IMAGE_CLIP_DIALOG_H
#define IMAGE_CLIP_DIALOG_H

#include <w_dialog.h>
#include "image_process_thread.h"

namespace Ui {
class ImageClipDialog;
}
namespace ady{
class ImageClipDialogPrivate;
class ImageClipDialog : public wDialog
{
    Q_OBJECT

public:

    ~ImageClipDialog();
    void initView();
    static ImageClipDialog* getInstance();
    static ImageClipDialog* open(QWidget* parent);

public slots:
    void onOk();
    void onFinishOne(int,int,const QString& source,const QString& destination);
    void onProressComplete();
    void onAddToWorkflow();

private:
    explicit ImageClipDialog(QWidget *parent = nullptr);

private:
    Ui::ImageClipDialog *ui;
    static ImageClipDialog* instance;
    ImageClipDialogPrivate *d;
};
}
#endif // IMAGE_CLIP_DIALOG_H
