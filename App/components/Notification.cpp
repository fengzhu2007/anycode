#include "Notification.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "ui_NotificationItem.h"
#include <QDebug>
namespace ady {
constexpr const  char NotificationItem::INFO_QSS[];
constexpr const  char NotificationItem::SUCCESS_QSS[];
constexpr const  char NotificationItem::WARNING_QSS[] ;
constexpr const  char NotificationItem::ERROR_QSS[] ;


    long long NotificationItem::seq = 1l;
    NotificationItem::NotificationItem(QWidget* parent)
        :QWidget(parent),
        ui(new Ui::NotificationItemUI)
    {
        ui->setupUi(this);
        ui->closeButton->setResources(QString::fromUtf8(":/img/Resource/close.svg"),QString::fromUtf8(":/img/Resource/close-hover.svg"));
        m_id = NotificationItem::seq++;
        m_status = Info;
        m_closeable = true;
        m_showtime = 0;
        connect(ui->closeButton,&IconWidget::clicked,this,&NotificationItem::onClose);
        connect(&m_timer,&QTimer::timeout,this,&NotificationItem::onClose);
    }

    void NotificationItem::setStatus(Status status)
    {
        m_status = status;
        switch(m_status){
            case Info:
                ui->frame->setStyleSheet(INFO_QSS);
                ui->icon->setPixmap(QPixmap(QString::fromUtf8(":/img/Resource/info-circle.svg")));
                break;
            case Success:
                ui->frame->setStyleSheet(SUCCESS_QSS);
                ui->icon->setPixmap(QPixmap(QString::fromUtf8(":/img/Resource/success-circle.svg")));
                break;
            case Warning:
                ui->frame->setStyleSheet(WARNING_QSS);
                ui->icon->setPixmap(QPixmap(QString::fromUtf8(":/img/Resource/warning-circle.svg")));
                break;
            case Error:
                ui->frame->setStyleSheet(ERROR_QSS);
                ui->icon->setPixmap(QPixmap(QString::fromUtf8(":/img/Resource/error-circle.svg")));
                break;
        }
    }

    void NotificationItem::setLabel(QString label)
    {
        ui->label->setText(label);
    }

    void NotificationItem::setCloseable(bool closeable)
    {
        ui->closeButton->setVisible(closeable);
    }

    void NotificationItem::setShowTime(qint64 ms)
    {
        m_showtime = ms;
        if(m_showtime>0){
            if(m_timer.isActive()){
                m_timer.stop();
            }
            m_timer.start(ms);
        }else{
            m_timer.stop();
        }
    }

    void NotificationItem::onClose()
    {
        Notification* notification = static_cast<Notification*>(this->parentWidget());
        int count = notification->layout()->count();
        for(int i=0;i<count;i++){
            if(notification->layout()->itemAt(i)->widget()==this){
                QLayoutItem* item = notification->layout()->takeAt(i);
                notification->layout()->removeItem(item);
                notification->resize();
                //delete item->widget();
                delete item;
                return ;
            }
        }
    }


    Notification::Notification(QWidget* parent)
        :QWidget(parent)
    {
        this->raise();
        QVBoxLayout* layout = new QVBoxLayout();
        this->setLayout(layout);
        layout->setSpacing(m_space);
        layout->setContentsMargins(QMargins(5,0,5,6));
    }


     NotificationItem* Notification::notify(QString text,NotificationItem::Status status,qint64 showtime,long long nid)
     {
        this->show();
        QLayout *layout = this->layout();
        if(nid>0){
            int count = layout->count();
            for(int i=0;i<count;i++){
                NotificationItem* item = (NotificationItem*)layout->itemAt(i)->widget();
                if(item->dataId()==nid){
                    item->setLabel(text);
                    item->setStatus(status);
                    item->setShowTime(showtime);
                    return item;
                }
            }
        }

        NotificationItem* item = new NotificationItem(this);
        item->setStatus(status);
        item->setShowTime(showtime);
        item->setLabel(text);
        layout->addWidget(item);
        this->resize();
        return item;
     }

     void Notification::resize()
     {
        int count = this->layout()->count();
        if(count==0){
            hide();
            return;
        }
        int height = 6;
        for(int i=0;i<count;i++){
            QRect rc = this->layout()->itemAt(0)->widget()->geometry();
            height += rc.height();
        }
        height += (m_space * (count -1));
        QWidget* parent = this->parentWidget();
        QRect rect = parent->geometry();
        QRect rc = this->geometry();
        rc.setX(0);
        rc.setWidth(rect.width());
        rc.setY(rect.height() - height);
        rc.setHeight(height);
        this->setGeometry(rc);
     }
}
