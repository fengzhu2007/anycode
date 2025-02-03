#include "image_view.h"
#include <QLabel>
#include <QImage>
#include <QPainter>
#include <QDebug>
namespace ady{

ImageContainer::ImageContainer(QWidget* parent):QLabel(parent){

}

void ImageContainer::paintEvent(QPaintEvent* e){
    QPainter p(this);
    auto size = this->size();
    int width = size.width();
    int height = size.height();
    for(int i=0;i<width;i+=20){
        for(int j=0;j<height;j+=20){
            p.fillRect(QRect(i,j,10,10),Qt::lightGray);
            p.fillRect(QRect(i+10,j,10,10),Qt::white);
            p.fillRect(QRect(i,j+10,10,10),Qt::white);
            p.fillRect(QRect(i+10,j+10,10,10),Qt::lightGray);
        }
    }
    QLabel::paintEvent(e);
}

class ImageViewPrivate{
public:
    ImageContainer* container;
    QImage* image;
    QSize imageSize;
    QSize originalSize;
    float zoom = 1.0f;
};

ImageView::ImageView(QWidget* parent):QScrollArea(parent) {
    d = new ImageViewPrivate;
    d->container = new ImageContainer(this);
    d->container->setAlignment(Qt::AlignCenter);
    d->image = nullptr;
    d->zoom = 1.0f;
    this->setStyleSheet("QScrollArea{border:0}");
    this->setWidget(d->container);
}

ImageView::~ImageView(){
    if(d->image){
        delete d->image;
    }
    delete d;
}

void ImageView::setImagePath(const QString& path){
    if(d->image){
        delete d->image;
        d->image = nullptr;
    }
    d->zoom  = 1.0f;
    d->image = new QImage(path);
    d->imageSize = d->originalSize = d->image->size();
    d->container->setPixmap(QPixmap::fromImage(*d->image));
    auto size = this->viewport()->size();
    d->container->setGeometry(QRect(0,0,d->imageSize.width(),d->imageSize.height()));
    this->viewImage(d->imageSize,size);
}

QSize ImageView::imageSize(){
    return d->imageSize;
}

QSize ImageView::originalSize(){
    return d->originalSize;
}

void ImageView::setZoom(float zoom){
    d->zoom = zoom;
    auto image = d->image->scaled(d->originalSize.width() * d->zoom,d->originalSize.height() * d->zoom);
    d->imageSize = image.size();
    d->container->setPixmap(QPixmap::fromImage(image));
    auto size = this->viewport()->size();
    this->viewImage(d->imageSize,size);
}

float ImageView::zoom(){
    return d->zoom;
}

void ImageView::showEvent(QShowEvent* e){
    this->viewImage(d->imageSize,this->size());
    QScrollArea::showEvent(e);
}

void ImageView::viewImage(const QSize& imageSize,const QSize& size){
    int width = imageSize.width() > size.width() ? imageSize.width():size.width() - 2;
    int height = imageSize.height() > size.height() ? imageSize.height():size.height() - 2;
    d->container->setGeometry(QRect(0,0,width,height));
}


}
