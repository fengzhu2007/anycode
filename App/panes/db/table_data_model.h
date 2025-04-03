#ifndef TABLE_DATA_MODEL_H
#define TABLE_DATA_MODEL_H
#include <QAbstractListModel>
#include <QStringList>
#include <QSqlField>
namespace ady{

class TableDataModelPrivate;
class TableDataModel : public QAbstractListModel
{
public:
    explicit TableDataModel(QObject* parent=nullptr);
    ~TableDataModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,int role = Qt::DisplayRole) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

    void setDatasource(const QList<QSqlField>& fields,const QList<QList<QVariant>>& data);

private:
    TableDataModelPrivate* d;

};
}
#endif // TABLE_DATA_MODEL_H
