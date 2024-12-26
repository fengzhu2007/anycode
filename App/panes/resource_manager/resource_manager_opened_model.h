#ifndef RESOURCE_MANAGER_OPENED_MODEL_H
#define RESOURCE_MANAGER_OPENED_MODEL_H

#include <QAbstractListModel>
#include <QItemDelegate>
#include "resource_manage_icon_provider.h"

namespace ady{
class ResourceManagerOpenedModel : public QAbstractListModel
{
public:
    static void init(QObject *parent);
    static ResourceManagerOpenedModel* getInstance();
    void setDataSource(const QStringList& data);

    inline QPair<QString,QString> itemAt(int row) const{
        return m_data.at(row);
    }

    void appendItem(const QString& path);
    void removeItem(const QString& path);
    void updateItem(const QString& oldPath,const QString& newPath);

protected:
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
private:
    explicit ResourceManagerOpenedModel(QObject *parent = nullptr);

private:
    QList<QPair<QString,QString>> m_data;
    ResourceManageIconProvider* m_provider;
    static ResourceManagerOpenedModel* instance;



};



class ResourceManagerOpenedItemDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    ResourceManagerOpenedItemDelegate(QObject* parent);
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override ;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
private:
    void drawText(QPainter *painter,const QStyleOptionViewItem &option,const QRect &rect,const QModelIndex &index) const;
};



}
#endif // RESOURCE_MANAGER_OPENED_MODEL_H
