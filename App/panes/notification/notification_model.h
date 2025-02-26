#ifndef NOTIFICATION_MODEL_H
#define NOTIFICATION_MODEL_H

#include "global.h"
#include "components/listview/listview_model.h"
#include "core/event_bus/event_data.h"

namespace ady{

class NotificationModelPrivate;
class ANYENGINE_EXPORT NotificationModel : public ListViewModel{
    Q_OBJECT
public:
    NotificationModel(ListView* parent);
    virtual int count() override;
    virtual ListViewItem* item(int i)  override;
    virtual void itemRemoved(int i) override;
    void setDataSource(QList<NotificationData> list);
    void appendItem(const NotificationData& item);
    NotificationData itemAt(int i);

signals:
    void itemClicked(int i);
private:
    NotificationModelPrivate* d;

};

}

#endif // NOTIFICATION_MODEL_H
