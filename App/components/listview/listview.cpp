#include "listview.h"
#include "listview_model.h"
#include <QMouseEvent>
#include <QLabel>
#include <QScrollBar>
#include <QStyleOption>
#include <QPainter>
#include <QDebug>
namespace ady{

class ListViewContentLayoutPrivate{
public:
    QList<QLayoutItem* >items;

};


ListViewContentLayout::ListViewContentLayout(QWidget* parent, int margin, int spacing)
    :QLayout(parent){
    setMargin(margin);
    setSpacing(spacing);
    d = new ListViewContentLayoutPrivate;
}

void ListViewContentLayout::addItem(QLayoutItem *item)
{
    d->items.push_back(item);
}



Qt::Orientations ListViewContentLayout::expandingDirections() const
{
    return Qt::Horizontal | Qt::Vertical;
}

bool ListViewContentLayout::hasHeightForWidth() const
{
    return false;
}

int ListViewContentLayout::count() const
{
    //return QLayout::count();
    return d->items.count();
}

QLayoutItem *ListViewContentLayout::itemAt(int index) const
{
    if(index>=0 && index<d->items.length()){
        return d->items.at(index);
    }else{
        return nullptr;
    }

}

QSize ListViewContentLayout::minimumSize() const
{
    /*int n = this->count();
    int h = 0;
    for(int i=0;i<n;i++){
        QLayoutItem* item = itemAt(i);
        QSize size = item->minimumSize();
        h += size.height();
    }
    return QSize(100,h);*/
    return this->sizeHint();
}

void ListViewContentLayout::setGeometry(const QRect &rect)
{
    int sp = this->spacing();
    int n = this->count();
    int x = sp;
    int y = sp;
    int w = 0;
    int h = 0;
    QRect rc = parentWidget()->geometry();
    w = rc.width() - sp * 2;
    for(int i=0;i<n;i++){
        QLayoutItem* item = itemAt(i);
        QSize size = item->widget()->sizeHint();
        auto maxSize = item->widget()->maximumSize();
        size.setHeight(std::min(size.height(),maxSize.height()));
        size.setWidth(std::min(size.width(),maxSize.width()));
        item->widget()->show();
        h = size.height();
        item->setGeometry(QRect(x,y,w,h));
        y+=h;
        y+=sp;
    }
}

QSize ListViewContentLayout::sizeHint() const
{
    int sp = this->spacing();
    int n = this->count();
    int h = sp;
    int w = 0;
    for(int i=0;i<n;i++){
        QLayoutItem* item = itemAt(i);
        QSize size = item->sizeHint();

        h += size.height();
        h+= sp;
        if(w<size.width()){
            w = size.width();
        }
    }
    return QSize(w,h);
}

QLayoutItem *ListViewContentLayout::takeAt(int index){
    return d->items.takeAt(index);
}

void ListViewContentLayout::addWidget(QWidget *w){
    int n = this->count();
    for(int i=0;i<n;i++){
        QLayoutItem* item = this->itemAt(i);
        if(item->widget()==w){
            return ;
        }
    }
    QLayout::addWidget(w);
}




/**************ListView start***************/

class ListViewPrivate{
public:
    ListViewModel* model = nullptr;
    int i=-1;
    bool multiple=false;
    QList<int> selected;

};


ListView::ListView(QWidget* parent)
    :QScrollArea(parent)
{

    d = new ListViewPrivate();
    QWidget* widget = this->widget();
    if(widget==nullptr){
        widget = new QWidget(this);
        this->setWidget(widget);
    }
}

ListView::~ListView(){
    delete d;
}

void ListView::clearSelected(){
    //d->selected.clear();
    if(d->selected.length()>0){
        foreach(int i,d->selected){
            ListViewItem* w = d->model->at(d->selected.at(0));
            if(w!=nullptr){
                w->setState(ListViewItem::Normal);
            }
        }
    }
}

void ListView::setModel(ListViewModel* model){
    d->model = model;
    this->render();
}

ListViewModel* ListView::model(){
    return d->model;
}


void ListView::render(){
    QWidget* widget = this->widget();
    ListViewContentLayout* layout = (ListViewContentLayout*)widget->layout();
    if(layout==nullptr){
        layout = new ListViewContentLayout(widget);
        widget->setLayout(layout);
    }
    int total = layout->count();
    if(d->model!=nullptr){
        int n = d->model->count();
        for(int i=0;i<n;i++){
            QWidget* w = d->model->item(i);
            layout->addWidget(w);
        }
        if(n<total){
            //remove
            for(int i=n;i<total;i++){
                ListViewItem* w = d->model->takeAt(n);
                QLayoutItem* item = layout->takeAt(n);
                w->close();
                w->deleteLater();
                delete item;
            }
        }
        QWidget* w = d->model->emptyWidget();
        if(n==0){
            //show no data
            w->show();
        }else{
            w->hide();
        }
        layout->update();
        if(n==0){
            this->resizeEmptyWidget();
        }
    }
}

void ListView::renderItem(int i){
    QWidget* widget = this->widget();
    ListViewContentLayout* layout = (ListViewContentLayout*)widget->layout();
    if(layout==nullptr){
        layout = new ListViewContentLayout(widget);
        widget->setLayout(layout);
    }
    if(d->model!=nullptr){
        QWidget* w = d->model->item(i);
        layout->addWidget(w);
        layout->update();
        d->model->emptyWidget()->hide();
    }
}

void ListView::removeItem(int i){
    QWidget* widget = this->widget();
    auto layout = static_cast<ListViewContentLayout*>(widget->layout());
    if(layout!=nullptr){
        QLayoutItem* item = layout->takeAt(i);
        delete item;
        if(d->model->count()==0){
            d->model->emptyWidget()->show();
        }
    }
}

void ListView::setSelection(const QList<int>& indexes){
    foreach(auto i,indexes){
        if(d->multiple==false){
            if(d->selected.size()>0){
                ListViewItem* w = d->model->at(d->selected.at(0));
                if(w!=nullptr){
                    w->setState(ListViewItem::Normal);
                }
            }
            d->selected.clear();
            if(!d->selected.contains(i)){
                d->selected.push_back(i);
                ListViewItem* w = d->model->at(i);
                if(w!=nullptr){
                    w->setState(ListViewItem::Selected);
                }
            }
        }else{
            //
            if(d->selected.contains(i)){
                //remove selected state
                d->selected.removeOne(i);
                ListViewItem* w = d->model->item(i);
                w->setState(ListViewItem::Normal);
            }else{
                //add seelcted state
                d->selected.append(i);
                ListViewItem* w = d->model->item(i);
                w->setState(ListViewItem::Selected);
            }
        }
    }
}

QList<int> ListView::selection(){
    return d->selected;
}

void ListView::mousePressEvent(QMouseEvent *e){
    if(e->button()==Qt::LeftButton)
        d->i = this->findItem(e->pos());
}

void ListView::mouseMoveEvent(QMouseEvent *e) {

}

void ListView::mouseReleaseEvent(QMouseEvent *e){
    int i = this->findItem(e->pos());
    if(i>=0 && i==d->i){
        //set selected
        this->setSelection(QList<int>{i});
        emit itemClicked(d->i);
    }
    d->i = -1;
}

void ListView::resizeEvent(QResizeEvent *event){
    QScrollArea::resizeEvent(event);
    this->resizeEmptyWidget();
}

void ListView::resizeEmptyWidget(){
    if(d->model!=nullptr && d->model->count()==0){
        QWidget* w = d->model->emptyWidget();
        if(w && w->isHidden()==false){
            QRect rc = this->geometry();
            w->setGeometry(QRect(0,10,rc.width() - 2,w->sizeHint().height()));
        }
    }
}


int ListView::findItem(const QPoint& pos){
    ListViewContentLayout* layout = (ListViewContentLayout*)this->widget()->layout();
    if(layout!=nullptr){
        int n = layout->count();
        QScrollBar* bar = this->verticalScrollBar();
        int offsetY = 0;
        if(bar!=nullptr){
            offsetY = bar->value();
        }
        for(int i=0;i<n;i++){
            QLayoutItem *item = layout->itemAt(i);
            if(item->geometry().contains(pos.x(),pos.y() + offsetY)){
                return i;
            }
        }
    }
    return -1;
}

}
