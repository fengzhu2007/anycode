#ifndef QUERY_RESULT_MODEL_H
#define QUERY_RESULT_MODEL_H
#include <QAbstractTableModel>
#include <QDateTime>
namespace ady{
class QueryResultModelPrivate;

class QueryResult{
public:

    QString domain;
    QDateTime startDate;
    QDateTime expireDate;
    QString errorMsg;

};

class QueryResultModel : public QAbstractTableModel
{
public:
    enum Column {
        Domain=0,
        StartDate,
        ExpireDate,
        ErrorMsg,
        Max
    };
    QueryResultModel(QObject *parent = nullptr);
    ~QueryResultModel();
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation,int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    void setDataSource(const QList<QueryResult>& list);

    void appendItem(const QueryResult& item);
    void removeItem(const QueryResult& item);
    void removeItem(const QString& domain);
    void updateItem(const QueryResult& item);


private:
    QueryResultModelPrivate* d;
};
}
#endif // QUERY_RESULT_MODEL_H
