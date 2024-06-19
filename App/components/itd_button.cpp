#include "itd_button.h"
#include <QStyleOption>
#include <QPainter>
#include <QMouseEvent>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSvgRenderer>

namespace ady{
class ITDButtonPrivate{
public:
    bool clicked=false;
    QLabel* icon;
    QLabel* title;
    QLabel* description;


};

ITDButton::ITDButton(QWidget* parent)
    :QFrame(parent)
{
    this->setStyleSheet("ady--ITDButton{background-color:#eeebeb;border:1px solid #eeebeb;}"
                        "ady--ITDButton[state='hover']{background-color:#c9def5}"
                        "ady--ITDButton QLabel#title{font-size:16px;color:#333}"
                        "ady--ITDButton QLabel#description{font-size:12px;color:#999}"
                        "ady--ITDButton QLabel#icon{width:32px;height:32px;}");
    d = new ITDButtonPrivate;
    d->icon = new QLabel(this);
    d->icon->setObjectName("icon");
    d->title = new QLabel(this);
    d->title->setObjectName("title");
    d->description = new QLabel(this);
    d->description->setObjectName("description");
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addSpacing(20);
    layout->setContentsMargins(0,10,20,10);
    this->setLayout(layout);

    QVBoxLayout* vLayout = new QVBoxLayout();
    vLayout->setContentsMargins(0,0,0,0);
    vLayout->setSpacing(8);

    vLayout->addWidget(d->title);
    vLayout->addWidget(d->description);
    layout->addWidget(d->icon);
    layout->addLayout(vLayout,1);
}

ITDButton::~ITDButton(){
    delete d;
}


void ITDButton::setIcon(const QString& src){
    if(!src.endsWith(".svg")){
        d->icon->setPixmap(QPixmap(src));
    }else{
        QSvgRenderer svgRenderer(src);
        QSize targetSize(32, 32);
        QPixmap pixmap(targetSize);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        svgRenderer.render(&painter);
        d->icon->setPixmap(pixmap);
    }
}

void ITDButton::setText(const QString& title){
    d->title->setText(title);
}

void ITDButton::setDescription(const QString& description){
    d->description->setText(description);
}


void ITDButton::enterEvent(QEvent *event){
    QFrame::enterEvent(event);
    this->setProperty("state","hover");
    this->style()->polish(this);
}

void ITDButton::leaveEvent(QEvent *event){
    QFrame::leaveEvent(event);
    this->setProperty("state","");
    this->style()->polish(this);
}

void ITDButton::mousePressEvent(QMouseEvent *event){
    QFrame::mousePressEvent(event);
    d->clicked = true;
}

void ITDButton::mouseMoveEvent(QMouseEvent *event){
    QFrame::mouseMoveEvent(event);
    QPoint pos = event->pos();
    int x = pos.x();
    int y = pos.y();
    QRect rect = this->rect();
    if(x<=0 || y<=0 || x>rect.width() || y>rect.height()){
        //out
        //setState(wSystemButton::Hover);
        d->clicked = false;
    }
    event->accept();
}

void ITDButton::mouseReleaseEvent(QMouseEvent *event){
    QFrame::mouseReleaseEvent(event);
    //setState(wSystemButton::Normal);
    if(d->clicked){
        emit clicked();
    }
    d->clicked = false;
}

void ITDButton::paintEvent(QPaintEvent *e)
{
    //QFrame::paintEvent(e);
    Q_UNUSED(e);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Frame, &opt, &p, this);
}
}
