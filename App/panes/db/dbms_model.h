#ifndef DBMS_MODEL_H
#define DBMS_MODEL_H
#include "global.h"
#include <QAbstractItemModel>

namespace ady{
class DBMSModelItemPrivate;
class DBMSModelItem{
public:
    enum Type{
        Solution=0,
        Connection,
        DatabaseItem,
        ItemType,//table typename view typename
        Table,//table
        View,
    };
    DBMSModelItem();
    DBMSModelItem(Type type,long long id,const QString& driver,const QString& name,DBMSModelItem* parent);//Connection id and connect name
    DBMSModelItem(Type type,const QString& name,DBMSModelItem* parent);//DatabaseItem
    DBMSModelItem(Type type,int itemType,const QString& name,DBMSModelItem* parent);//itemtype or item


    ~DBMSModelItem();

    int childrenCount();
    int row();
    void appendItems(QList<DBMSModelItem*> items);
    void appendItem(DBMSModelItem* item);
    void insertItem(int row,DBMSModelItem* item);
    DBMSModelItem* parent();
    DBMSModelItem* childAt(int i);
    DBMSModelItem* take(int i);
    QList<DBMSModelItem*> takeAll();
    Type type();
    QString name() const;
    long long id() const ;
    long long pid() const;
    bool status() const;
    int itemType() const;
    void setName(const QString& name);
    void setStatus(bool status);
    bool expanded();
    void setExpanded(bool expanded);
    bool isLoading();
    void setLoading(bool state);
    DBMSModelItem* findChild(long long id);


private:
    DBMSModelItemPrivate* d;
};

class DBRecord;
class DBMSModelPrivate;
class ANYENGINE_EXPORT DBMSModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum Column {
        Name=0,
        Max
    };
    explicit DBMSModel(QObject *parent = nullptr);
    ~DBMSModel();
    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    // Basic functionality:
    QModelIndex index(int row, int column,const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

    void appendItem(DBMSModelItem* parent,DBMSModelItem* item);
    void setDatasource(const QList<DBRecord>& list);
    void addConnection(long long id,const QString& driver,const QString& name);
    void updateConnection(long long id,const QString& name);
    void updateConnection(long long id,bool status);
    void removeConnection(long long id);
    void removeChildren(DBMSModelItem* item);

    void addDatabase(const QStringList& list,DBMSModelItem* parent);
    void updateDatabase(const QModelIndex& index,bool status);

    void addType(const QList<QPair<int,QString>> list,DBMSModelItem* parent);
    void addTable(const QStringList& list,DBMSModelItem* parent);
    void addView(const QStringList& list,DBMSModelItem* parent);





    void removeItem(DBMSModelItem* item);


    DBMSModelItem* rootItem();

    //void dataChanged();
    void changeItem(DBMSModelItem* item);

signals:
    void rename(DBMSModelItem* item,const QString& newName);

private:
    DBMSModelPrivate * d;
};
}
#endif // DBMS_MODEL_H
