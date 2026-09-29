#include "notification_card.h"
#include "ui_notification_card.h"
#include <QStyleOption>
#include <QPainter>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QTextDocument>
namespace ady{

class NotificationCardPrivate{
public:
    Ui::NotificationCard * ui;
    NotificationData data;
};


NotificationCard::NotificationCard(QWidget *parent)
    : ListViewItem{parent}
{
    d = new NotificationCardPrivate;
    d->ui = nullptr;
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this,&QWidget::customContextMenuRequested,this,&NotificationCard::onContextMenu);
}

void NotificationCard::init(const NotificationData& data){
    if(!d->ui){
        d->ui = new Ui::NotificationCard;
        d->ui->setupUi(this);
    }
    d->data = data;
    d->ui->title->setText(data.title);
    d->ui->description->setText(data.description);
    d->ui->time->setText(data.time);
}

void NotificationCard::onContextMenu(const QPoint& pos){
    QMenu menu(this);
    if(!d->data.copyText.isEmpty()){
        menu.addAction(tr("Copy"),this,[this](){
            QApplication::clipboard()->setText(d->data.copyText);
        });
    }
    menu.addAction(tr("Copy Text"),this,[this](){
        //description may contain html tags,convert to plain text before copying
        QTextDocument doc;
        doc.setHtml(d->data.description);
        QApplication::clipboard()->setText(doc.toPlainText());
    });
    menu.exec(this->mapToGlobal(pos));
}

void NotificationCard::paintEvent(QPaintEvent *e){
    Q_UNUSED(e);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Frame, &opt, &p, this);
}

}
