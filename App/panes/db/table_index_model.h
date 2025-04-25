#ifndef TABLE_INDEX_MODEL_H
#define TABLE_INDEX_MODEL_H


#include <QAbstractListModel>
#include "table_field.h"


namespace ady{
class TableIndexModelPrivate;
class TableIndexModel : public QAbstractListModel
{
public:
    enum Column{
        Name=0,
        FeildList,
        Max
    };
    explicit TableIndexModel(QObject* parent=nullptr);
    ~TableIndexModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,int role = Qt::DisplayRole) const override;

    void setDatasource(const QList<TableIndex>& data);
    void appendItem(const TableIndex& field);
    void updateItem(const TableIndex& field);
    void removeItem(int row);

    QList<TableIndex>& indexes() const;


    TableIndex at(int row) const ;

private:
    TableIndexModelPrivate* d;


};
}



#endif // TABLE_INDEX_MODEL_H
