#ifndef IMAGE_VIEW_H
#define IMAGE_VIEW_H
#include "global.h"
#include <QScrollArea>
namespace ady{
class ImageViewPrivate;
class ANYENGINE_EXPORT ImageView : public QScrollArea
{
public:
    explicit ImageView(QWidget* parent=nullptr);
    ~ImageView();
    void setImagePath(const QString& path);
    QSize imageSize();

protected:
    virtual void showEvent(QShowEvent* e) override;

private:
    void viewImage(const QSize& size);

private:
    ImageViewPrivate* d;
};
}
#endif // IMAGE_VIEW_H
