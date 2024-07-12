#ifndef COMMIT_MODEL_H
#define COMMIT_MODEL_H
#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include "commit.h"
namespace ady {

class CommitItemDelegatePrivate;
class CommitItemDelegate : public QStyledItemDelegate
{
public:
    CommitItemDelegate(QObject* parent=nullptr);
    ~CommitItemDelegate();
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    CommitItemDelegatePrivate* d;


};



class CommitModelPrivate;
class CommitModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Column {
        //Flags=0,
        DateTime=0,
        Author,
        Content,
        Max
    };
    CommitModel(QObject* parent=nullptr);
    ~CommitModel();

    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,int role = Qt::DisplayRole) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    cvs::Commit getItem(int pos);

    void setList(QList<cvs::Commit> data);
    cvs::Commit at(int row);
    void setDataSource(QList<cvs::Commit> data);
    void appendList(QList<cvs::Commit> data);
    void updateFlags(QList<cvs::Commit> data);
    //void updateFlags(int top,int bottom);
    //void removeFlags(QList<cvs::Commit> data);

    QList<cvs::Commit> compareRows();
    int compareRow(int row);
    void clearCompareRows();



private:
    CommitModelPrivate* d;


};

}

#endif // COMMIT_MODEL_H
