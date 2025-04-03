#ifndef TABLE_FIELD_MODEL_H
#define TABLE_FIELD_MODEL_H


#include <QAbstractListModel>
#include <QSqlField>


namespace ady{
class TableFieldModelPrivate;
class TableFieldModel : public QAbstractListModel
{
public:
    enum Column{
        Name=0,
        Type,
        Length,
        PrimaryKey,
        Max
    };
    explicit TableFieldModel(QObject* parent=nullptr);
    ~TableFieldModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,int role = Qt::DisplayRole) const override;

    void setDatasource(const QList<QSqlField>& data);

private:
    TableFieldModelPrivate* d;


};
}
#endif // TABLE_FIELD_MODEL_H
