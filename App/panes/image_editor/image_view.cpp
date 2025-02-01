#include "image_view.h"
#include <QLabel>
#include <QImage>
#include <QDebug>
namespace ady{
class ImageViewPrivate{
public:
    QLabel* image;
    QSize imageSize;
};

ImageView::ImageView(QWidget* parent):QScrollArea(parent) {
    d = new ImageViewPrivate;
    d->image = new QLabel(this);
    d->image->setAlignment(Qt::AlignCenter);
    //this->takeWidget();
    //this->setStyleSheet("border:0");
    this->setStyleSheet("QScrollArea{border:0}");
    this->setWidget(d->image);
   // auto widget = this->widget();

}

ImageView::~ImageView(){
    delete d;
}

void ImageView::setImagePath(const QString& path){
    QImage image(path);
    d->imageSize = image.size();
    d->image->setPixmap(QPixmap::fromImage(image));
    auto size = this->viewport()->size();
    d->image->setGeometry(QRect(0,0,d->imageSize.width(),d->imageSize.height()));
    this->viewImage(size);
}

QSize ImageView::imageSize(){
    return d->imageSize;
}

void ImageView::showEvent(QShowEvent* e){
    this->viewImage(this->size());
    QScrollArea::showEvent(e);
}

void ImageView::viewImage(const QSize& size){
    int width = d->imageSize.width() > size.width() ? d->imageSize.width():size.width() - 2;
    int height = d->imageSize.height() > size.height() ? d->imageSize.height():size.height() - 2;

    d->image->setGeometry(QRect(0,0,width,height));
}
}
