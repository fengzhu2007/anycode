#include "addon_manager_model.h"
#include "addon_item_widget.h"
#include "components/listview/listview.h"
namespace ady{

AddonManagerModel::AddonManagerModel(ListView* parent):ListViewModel(parent){

}

ListViewItem* AddonManagerModel::item(int i){
    AddonItemWidget* w = (AddonItemWidget*)ListViewModel::item(i);
    if(w==nullptr){
        w = new AddonItemWidget(this->listView()->widget());
        ListViewModel::addWidget(w);
    }
    AddonItem one = this->itemAt(i);

    w->setAddonItem(one);
    return w;
}

void AddonManagerModel::itemRemoved(int i){
    m_list.takeAt(i);
    ListViewModel::itemRemoved(i);
}

void AddonManagerModel::setDataSource(QList<AddonItem> list){
    m_list = list;
    this->dataChanged();
}

AddonItem AddonManagerModel::itemAt(int i){
    return m_list.at(i);
}



}
