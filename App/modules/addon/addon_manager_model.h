#ifndef ADDON_MANAGER_MODEL_H
#define ADDON_MANAGER_MODEL_H

#include "components/listview/listview_model.h"
#include <QIcon>

namespace ady{

class AddonItem{
public:
    bool installed;
    QString title;
    QString description;
    QString author;
    QString url;
    QString version;
    QIcon icon;
};



class ANYENGINE_EXPORT AddonManagerModel : public ListViewModel{
public:
    AddonManagerModel(ListView* parent);
    virtual int count() override {return m_list.size();}
    virtual ListViewItem* item(int i)  override;
    virtual void itemRemoved(int i) override;
    void setDataSource(QList<AddonItem> list);
    AddonItem itemAt(int i);

private:
    QList<AddonItem> m_list;

};



}

#endif // ADDON_MANAGER_MODEL_H
