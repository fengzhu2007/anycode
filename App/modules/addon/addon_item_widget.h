#ifndef ADDON_ITEM_WIDGET_H
#define ADDON_ITEM_WIDGET_H

#include <QWidget>
#include "components/listview/listview_model.h"

namespace Ui {
class AddonItemWidget;
}

namespace ady{
class AddonItem;
class AddonItemWidget : public ListViewItem
{
    Q_OBJECT

public:
    explicit AddonItemWidget(QWidget *parent = nullptr);
    ~AddonItemWidget();
    void setTitle(const QString& title);
    void setIcon(const QIcon& icon);
    void setDescription(const QString& description);
    void setInstalled(bool installed);
    void setAddonItem(const AddonItem& item);
    inline void setIndex(int index){m_index = index;}

public slots:
    void onInstall();
    void onUnInstall();
    void onDisable();

private:
    Ui::AddonItemWidget *ui;
    int m_index;
};
}

#endif // ADDON_ITEM_WIDGET_H
