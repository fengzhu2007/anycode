#include "IconWidget.h"
#include <QPixmap>
namespace ady{
IconWidget::IconWidget(QWidget* parent)
    :QLabel(parent)
{
    //m_normalPixmap = const_cast<QPixmap*>(this->pixmap());
    //m_hoverPixmap = nullptr;
    m_pressed = false;
}



void IconWidget::setResources(QString normalRes,QString hoverRes)
{
    m_normalRes = normalRes;
    m_hoverRes = hoverRes;
    setCursor(Qt::PointingHandCursor);
}

void IconWidget::enterEvent(QEvent *event)
{
    if(m_hoverRes.isEmpty()==false){
        setPixmap(QPixmap(m_hoverRes));
    }
    QLabel::enterEvent(event);
}

void IconWidget::leaveEvent(QEvent *event)
{
    m_pressed = false;
    if(m_normalRes.isEmpty()==false){
        setPixmap(QPixmap(m_normalRes));
    }
    QLabel::leaveEvent(event);
}

void IconWidget::mousePressEvent(QMouseEvent *event)
{
    m_pressed = true;
    QLabel::mousePressEvent(event);
}

void IconWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(m_pressed){
        emit clicked();
    }
    QLabel::mouseReleaseEvent(event);
}

}
