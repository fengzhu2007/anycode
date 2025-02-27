#ifndef NOTIFICATION_CARD_H
#define NOTIFICATION_CARD_H

#include "global.h"
#include "components/listview/listview_model.h"
#include "core/event_bus/event_data.h"
#include <QWidget>

namespace ady{
class NotificationCardPrivate;

class ANYENGINE_EXPORT NotificationCard : public ListViewItem
{
    Q_OBJECT
public:
    explicit NotificationCard(QWidget *parent = nullptr);
    virtual void init(const NotificationData& data);

protected:
    virtual void paintEvent(QPaintEvent *e) override;

private:
    NotificationCardPrivate* d;
};

}

#endif // NOTIFICATION_CARD_H
