#ifndef IMAGE_VIEW_H
#define IMAGE_VIEW_H
#include "global.h"
#include <QScrollArea>
#include <QLabel>
namespace ady{
class ANYENGINE_EXPORT ImageContainer : public QLabel{
public:
    explicit ImageContainer(QWidget* parent);

protected:
    virtual void paintEvent(QPaintEvent* e) override;
};


class ImageViewPrivate;
class ANYENGINE_EXPORT ImageView : public QScrollArea
{
public:
    explicit ImageView(QWidget* parent=nullptr);
    ~ImageView();
    void setImagePath(const QString& path);
    QSize imageSize();
    QSize originalSize();
    void setZoom(float zoom);
    float zoom();

protected:
    virtual void showEvent(QShowEvent* e) override;

private:
    void viewImage(const QSize& imageSize,const QSize& size);

private:
    ImageViewPrivate* d;
};
}
#endif // IMAGE_VIEW_H
