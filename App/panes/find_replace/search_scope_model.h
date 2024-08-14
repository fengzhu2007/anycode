#ifndef SEARCHSCOPEMODEL_H
#define SEARCHSCOPEMODEL_H
#include <QAbstractListModel>


namespace ady{
class SearchScopeModelPrivate;
class SearchScopeModel : public QAbstractListModel
{
public:
    enum Column{
        Name=0,
        Max
    };
    explicit SearchScopeModel(QObject* parent=nullptr);
    ~SearchScopeModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,int role = Qt::DisplayRole) const override;
    void setDataSource(QList<QPair<QString,QString>> list);
    QList<QPair<QString,QString>> toList();
    QString value(int i);
    //void setList(QList<AddonRecord> data);
    //inline AddonRecord dataIndex(int index){return this->m_data.at(index);}
    //int indexOf(long long  id);


private:
    SearchScopeModelPrivate* d;

};
}
#endif // SEARCHSCOPEMODEL_H
