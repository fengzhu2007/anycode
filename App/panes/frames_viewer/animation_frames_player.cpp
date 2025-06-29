#include "animation_frames_player.h"
#include "global.h"
#include <QLabel>
#include <QResizeEvent>
#include <QPainter>
#include <QDebug>
namespace ady{

class ANYENGINE_EXPORT Player : public QWidget{
public:
    explicit Player(QWidget* parent){
        this->imageViewer = new QLabel(this);
    }
    void load(const QString& path){

    }

    void load(const QPixmap& image){
        qDebug()<<"image"<<image;
        qDebug()<<"load"<<image.size();
        this->imageSize = image.size();
        this->imageViewer->setPixmap(image);

        //this->adjustSize(this->imageSize);
        this->imageViewer->setFixedSize(this->imageSize);
        this->imageViewer->setGeometry({0,0,this->imageSize.width(),this->imageSize.height()});

        qDebug()<<this->imageViewer->geometry();
    }

    void adjustSize(const QSize& cSize){
        auto size = this->imageViewer->size();
        qDebug()<<"image size"<<size;
        //auto cSize = e->size();
        int x = 0;
        int y = 0;
        if(cSize.width() > size.width()){
            x = (cSize.width() - size.width()) / 2;
        }
        if(cSize.height() > size.height()){
            y = (cSize.height() - size.height()) / 2;
        }

        this->imageViewer->setGeometry({x,y,size.width(),size.height()});
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
        //this->adjustSize(e->size());
    }




private:
    QLabel *imageViewer;
    QSize imageSize;


    friend class AnimationFramesPlayer;
};





class AnimationFramesPlayerPrivate{

public:
    Player* player;

};

AnimationFramesPlayer::AnimationFramesPlayer(QWidget *parent)
    : QScrollArea{parent}
{

    d = new AnimationFramesPlayerPrivate;
    d->player = new Player(this);
    //d->container->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    //d->container->setAlignment(Qt::AlignCenter);
    //d->zoom = 1.0f;
    this->setStyleSheet("QScrollArea{border:0}");
    this->setWidget(d->player);


}

AnimationFramesPlayer::~AnimationFramesPlayer(){
    delete d;
}

void AnimationFramesPlayer::load(const QPixmap& image){
    d->player->load(image);
}

void AnimationFramesPlayer::resizeEvent(QResizeEvent* e){
    QScrollArea::resizeEvent(e);
    int w = static_cast<int>(d->player->imageSize.width() * 1);
    int h = static_cast<int>(d->player->imageSize.height() * 1);
    int width = qMax(w,e->size().width());
    int height = qMax(h,e->size().height());
    auto rc = d->player->geometry();
    qDebug()<<"player resize:"<<QSize{w,h};
    d->player->imageViewer->setFixedSize({w,h});
    d->player->setGeometry({rc.x(),rc.y(),width,height});
    d->player->adjustSize(d->player->size());

}




}
