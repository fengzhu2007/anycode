#include "listview_model.h"
#include "listview.h"
#include <QList>
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
    qDebug()<<"setState:"<<state;
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

}
