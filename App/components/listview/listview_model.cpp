#include "listview_model.h"
#include "listview.h"
#include <QList>
#include <QLabel>
#include <QStyleOption>
#include <QPainter>
#include <QDebug>

namespace ady{


class ListViewItemPrivate{
public:
    ListViewItem::State state;

};

ListViewItem::ListViewItem(QWidget* parent)
    :QFrame(parent){
    d = new ListViewItemPrivate;
    d->state = Normal;
}

ListViewItem::~ListViewItem(){
    delete d;
}

void ListViewItem::setState(State state){
    //qDebug()<<"setState:"<<state;
    if(state!=d->state){
        d->state = state;
        //update style
        this->setProperty("state",(int)state);
        QStyle* style = this->style();
        style->polish(this);
    }
}

void ListViewItem::paintEvent(QPaintEvent *e){
    Q_UNUSED(e);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Frame, &opt, &p, this);
}

class ListViewModelPrivate{
public:
    QList<ListViewItem*>widgets;
    ListView* listView;
    QWidget* empty=nullptr;
};


ListViewModel::ListViewModel(ListView *parent)
    : QObject((QObject*)parent)
{
    d = new ListViewModelPrivate;
    d->listView = parent;
}

ListViewItem* ListViewModel::item(int i){
    if(i>=0 && i<d->widgets.size()){
        return d->widgets.at(i);
    }else{
        return nullptr;
    }
}

ListViewItem* ListViewModel::at(int i){
    if(i>=0 && i<d->widgets.size()){
        return d->widgets.at(i);
    }else{
        return nullptr;
    }
}

void ListViewModel::addWidget(ListViewItem* widget){
    d->widgets.push_back(widget);
}

ListViewItem* ListViewModel::takeAt(int i){
    return d->widgets.takeAt(i);
}

void ListViewModel::dataChanged(){
    //auto listView = static_cast<ListView*>(parent());
    d->listView->render();
}

void ListViewModel::itemChanged(int i){
    d->listView->renderItem(i);

}

void ListViewModel::itemRemoved(int i){
    QWidget* widget = d->widgets.takeAt(i);
    d->listView->removeItem(i);
    widget->close();
    widget->deleteLater();
}

QWidget* ListViewModel::emptyWidget(){
    if(d->empty==nullptr){
        auto label = new QLabel(d->listView->widget());
        label->setAlignment(Qt::AlignCenter);
        label->setText(QString::fromUtf8("<img src=':/Resource/images/nodata.svg'/><br/><br/>%1").arg(tr("No Data")));
        d->empty = label;
        QRect rc = d->listView->geometry();
        d->empty->setGeometry(QRect(0,10,rc.width() - 2,label->sizeHint().height()));
    }
    d->empty->show();
    return d->empty;
}

ListView* ListViewModel::listView(){
    return d->listView;
}

}
