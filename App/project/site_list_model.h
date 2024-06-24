#ifndef SITELISTMODEL_H
#define SITELISTMODEL_H
#include "global.h"
#include "components/listview/listview_model.h"
#include "storage/SiteStorage.h"
#include <QFrame>
#include <QStringList>

namespace ady{

class SiteItemWidgetPrivate;
class ANYENGINE_EXPORT SiteItemWidget : public ListViewItem{
    Q_OBJECT
public:
    SiteItemWidget(QWidget* parent);
    ~SiteItemWidget();
    void setText(const QString& text);
    void setTags(QStringList tags);

private:
    SiteItemWidgetPrivate *d;
};


class SiteListModelPrivate;
class ANYENGINE_EXPORT SiteListModel : public ListViewModel
{
public:
    SiteListModel(ListView* parent);
    ~SiteListModel();
    virtual int count() override;
    virtual ListViewItem* item(int i)  override;
    virtual void itemRemoved(int i) override;
    void setDataSource(QList<SiteRecord> list);
    int index(const SiteRecord& record);
    void updateItem(const SiteRecord& record);
    void appendItem(const SiteRecord& record);
    SiteRecord itemAt(int i);
private:
    SiteListModelPrivate* d;
};
}

#endif // SITELISTMODEL_H
