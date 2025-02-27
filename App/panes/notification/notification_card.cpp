#include "notification_card.h"
#include "ui_notification_card.h"
#include <QStyleOption>
#include <QPainter>
namespace ady{

class NotificationCardPrivate{
public:
    Ui::NotificationCard * ui;
};


NotificationCard::NotificationCard(QWidget *parent)
    : ListViewItem{parent}
{
    d = new NotificationCardPrivate;
    d->ui = nullptr;
}

void NotificationCard::init(const NotificationData& data){
    if(!d->ui){
        d->ui = new Ui::NotificationCard;
        d->ui->setupUi(this);
    }
    d->ui->title->setText(data.title);
    d->ui->description->setText(data.description);
    d->ui->time->setText(data.time);
}

void NotificationCard::paintEvent(QPaintEvent *e){
    Q_UNUSED(e);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Frame, &opt, &p, this);
}

}
