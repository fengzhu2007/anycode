#include "site_list_model.h"
#include "components/listview/listview.h"
#include "storage/AddonStorage.h"
#include "storage/GroupStorage.h"
#include "w_tags.h"
#include <QHBoxLayout>

namespace ady{
class SiteItemWidgetPrivate{
public:
    QLabel* title;
    wTags* tags;
};


SiteItemWidget::SiteItemWidget(QWidget* parent)
    :ListViewItem(parent){

    d = new SiteItemWidgetPrivate;
    d->title = new QLabel(parent);
    d->tags = new wTags(parent);
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setMargin(10);
    layout->setSpacing(6);
    this->setLayout(layout);
    layout->addWidget(d->title);
    layout->addWidget(d->tags);
}

SiteItemWidget::~SiteItemWidget(){
    delete d;
}

void SiteItemWidget::setText(const QString& text){
    //d->tags->addTag(text);
    d->title->setText(text);
}

void SiteItemWidget::setTags(QStringList tags){
    d->tags->setTags(tags);
}




/*************SiteListModel start***************/

class SiteListModelPrivate{
public:
    QList<SiteRecord> list;
    ListView* listview;

    AddonStorage addonStorage;
    GroupStorage groupStorage;
};


SiteListModel::SiteListModel(ListView* parent):
    ListViewModel(parent)
{
    d = new SiteListModelPrivate;
    d->listview = parent;
}

SiteListModel::~SiteListModel(){
    delete d;
}

int SiteListModel::count(){
    return d->list.size();
}

ListViewItem* SiteListModel::item(int i){
    //qDebug()<<"item:"<<d->listview->widget();
    SiteItemWidget* w = (SiteItemWidget*)ListViewModel::item(i);
    if(w==nullptr){
        w = new SiteItemWidget(d->listview->widget());
        ListViewModel::addWidget(w);
        //w->setMinimumHeight(60);
    }
    SiteRecord one = d->list.at(i);
    w->setText(one.name);
    QStringList tags;
    AddonRecord addon = d->addonStorage.one(one.type);
    if(addon.id>0){
        tags.push_back(addon.title);
    }
    GroupRecord group = d->groupStorage.one(one.groupid);
    if(group.id>0){
        tags.push_back(group.name);
    }
    if(tags.length()>0){
        w->setTags(tags);
    }
    return w;
}

void SiteListModel::setDataSource(QList<SiteRecord> list){
    d->list = list;
    this->dataChanged();
}

SiteRecord SiteListModel::itemAt(int i){
    return d->list.at(i);
}


}