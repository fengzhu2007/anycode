#include "svg_viewer.h"
#include <QImage>
#include <QPainter>
#include <QSvgRenderer>
#include <QResizeEvent>
#include <QDebug>
namespace ady{

class ANYENGINE_EXPORT SVGContainer : public QWidget{
public:
    explicit SVGContainer(QWidget* parent){
        this->svg = new QSvgWidget(this);

    }
    void load(const QString& path){
        this->svg->load(path);
        this->svgSize = this->svg->renderer()->defaultSize();
    }

    void adjustSize(const QSize& cSize){
        auto size = this->svg->size();
        //qDebug()<<"svg size"<<size;
        //auto cSize = e->size();
        int x = 0;
        int y = 0;
        if(cSize.width() > size.width()){
            x = (cSize.width() - size.width()) / 2;
        }
        if(cSize.height() > size.height()){
            y = (cSize.height() - size.height()) / 2;
        }
        this->svg->setGeometry({x,y,size.width(),size.height()});
    }

protected:
    virtual void paintEvent(QPaintEvent* e) override{
        QWidget::paintEvent(e);
        QPainter p(this);
        auto size = this->size();
        //qDebug()<<"size"<<size;
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
    }

    virtual void resizeEvent(QResizeEvent* e) override{
        QWidget::resizeEvent(e);
        this->adjustSize(e->size());
    }




private:
    QSize svgSize;
    QSvgWidget *svg;


    friend class SVGViewer;
};



class SVGViewPrivate{
public:
    SVGContainer* container;
    QSize imageSize;
    float zoom = 1.0f;
};

SVGViewer::SVGViewer(QWidget* parent):QScrollArea(parent) {
    d = new SVGViewPrivate;
    d->container = new SVGContainer(this);
    //d->container->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    //d->container->setAlignment(Qt::AlignCenter);
    d->zoom = 1.0f;
    this->setStyleSheet("QScrollArea{border:0}");
    this->setWidget(d->container);
}

SVGViewer::~SVGViewer(){
    delete d;
}

void SVGViewer::setImagePath(const QString& path){
    d->zoom  = 1.0f;
    d->container->load(path);
    d->imageSize = d->container->svgSize;
}

QSize SVGViewer::imageSize(){
    return d->imageSize;
}

QSize SVGViewer::originalSize(){
    return d->container->svgSize;
}

void SVGViewer::setZoom(float zoom){
    //qDebug()<<"zoom"<<d->zoom<<zoom;
    d->zoom = zoom;
    d->imageSize = {static_cast<int>(d->container->svgSize.width() * d->zoom),
                    static_cast<int>(d->container->svgSize.height() * d->zoom)};

    d->container->svg->setFixedSize(d->imageSize);

    auto size = this->viewport()->size();
    if(size.width()  < d->imageSize.width()){
        size.setWidth(d->imageSize.width());
    }
    if(size.height()  < d->imageSize.height()){
        size.setHeight(d->imageSize.height());
    }
    d->container->setFixedSize(size);
    d->container->adjustSize(size);
    //this->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

}


float SVGViewer::zoom(){
    return d->zoom;
}

QSvgRenderer* SVGViewer::renderer(){
    return d->container->svg->renderer();
}


void SVGViewer::resizeEvent(QResizeEvent* e){
    QScrollArea::resizeEvent(e);
    int w = static_cast<int>(d->container->svgSize.width() * d->zoom);
    int h = static_cast<int>(d->container->svgSize.height() * d->zoom);
    int width = qMax(w,e->size().width());
    int height = qMax(h,e->size().height());
    auto rc = d->container->geometry();
    d->container->svg->setFixedSize({w,h});
    d->container->setGeometry({rc.x(),rc.y(),width,height});
    d->container->adjustSize(d->container->size());
   // this->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}



}
