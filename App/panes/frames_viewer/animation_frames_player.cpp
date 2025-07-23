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
        //qDebug()<<"image"<<image;
        //qDebug()<<"load"<<image.size();
        this->imageSize = image.size();
        this->imageViewer->setPixmap(image);


        this->imageViewer->setFixedSize(this->imageSize);
        //this->imageViewer->setGeometry({0,0,this->imageSize.width(),this->imageSize.height()});

        //qDebug()<<this->imageViewer->geometry()<<this->geometry();

        auto rc = QRect(0,0,this->rangeSize.width(),this->rangeSize.height());

        if(this->rangeSize.width()<this->imageSize.width()){
            rc.setWidth(this->imageSize.width());
        }
        if(this->rangeSize.height()<this->imageSize.height()){
            rc.setHeight(this->imageSize.height());
        }
        auto rect = this->geometry();
        if(rect.width()!=rc.width() || rect.height()!=rc.height()){
            this->setGeometry(rc);
            //qDebug()<<this->imageViewer->geometry()<<this->geometry()<<rc;
            this->adjustSize(this->rangeSize);
        }

    }

    void adjustSize(const QSize& cSize){
        auto size = this->imageViewer->size();
        //qDebug()<<"image size"<<size;
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
        //qDebug()<<"player size 1:"<<e->size();
        //this->adjustSize(e->size());
    }




private:
    QLabel *imageViewer;
    QSize imageSize;
    QSize rangeSize;
    // int scrollTop = 0;
    // int scrollLeft = 0;

    friend class AnimationFramesPlayer;
};





class AnimationFramesPlayerPrivate{

public:
    Player* player;
    QLabel* label;

};

AnimationFramesPlayer::AnimationFramesPlayer(QWidget *parent)
    : QScrollArea{parent}
{

    d = new AnimationFramesPlayerPrivate;
    d->player = new Player(this);
    d->label = new QLabel(this);
    d->label->setAlignment(Qt::AlignCenter);
    d->label->setStyleSheet("QLabel{background:rgba(0,0,0,160);color:#ffffff;padding:4px 10px}");
    d->label->hide();
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

void AnimationFramesPlayer::setText(const QString& text){
    if(d->label->isHidden()){
        d->label->show();
    }
    d->label->setText(text);
}

void AnimationFramesPlayer::resizeEvent(QResizeEvent* e){
    QScrollArea::resizeEvent(e);
    d->player->rangeSize = e->size();
    d->player->setGeometry({0,0,e->size().width(),e->size().height()});
    //d->label->move(10,e->size().height() - 30);

    d->label->setGeometry({10,e->size().height() - 10 - 20,200,20});
}

// void AnimationFramesPlayer::scrollContentsBy(int dx, int dy){
//     QScrollArea::scrollContentsBy(dx,dy);
//     qDebug()<<"scrollContentsBy"<<dx<<dy;
//     d->player->scrollLeft -= dx;
//     d->player->scrollTop -= dy;
// }




}
