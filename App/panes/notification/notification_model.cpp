#include "notification_model.h"
#include "notification_card.h"
#include "components/listview/listview.h"
namespace ady{


class NotificationModelPrivate{
public:
    QList<NotificationData> list;
    ListView* listview;
};

NotificationModel::NotificationModel(ListView* parent)
    :ListViewModel(parent){
    d = new NotificationModelPrivate;
    d->listview = parent;
}

int NotificationModel::count(){
    return d->list.size();
}

ListViewItem* NotificationModel::item(int i){
    auto w = static_cast<NotificationCard*>(ListViewModel::item(i));
    if(w==nullptr){
        w = new NotificationCard(d->listview->widget());
        ListViewModel::addWidget(w);
    }
    NotificationData one = d->list.at(i);
    w->init(one);
    return w;
}

void NotificationModel::itemRemoved(int i){
    d->list.takeAt(i);
    ListViewModel::itemRemoved(i);
}

void NotificationModel::setDataSource(QList<NotificationData> list){
    d->list = list;
    this->dataChanged();
}

void NotificationModel::appendItem(const NotificationData& item){
    d->list.append(item);
    this->dataChanged();
}

NotificationData NotificationModel::itemAt(int i){
    return d->list.at(i);
}

}
