#ifndef COMMITMODEL_H
#define COMMITMODEL_H
#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include "Commit.h"
namespace ady {


class CommitItemDelegate : public QStyledItemDelegate
{
public:
    CommitItemDelegate(QObject* parent=nullptr);

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    QPixmap m_devIcon;
    QPixmap m_prodIcon;
    QPixmap m_otherIcon;
    int m_boxWidth =20;

};




class CommitModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Column {
        Flags=0,
        DateTime,
        Author,
        Content,
        Max
    };
    CommitModel(QObject* parent=nullptr);

    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,int role = Qt::DisplayRole) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    cvs::Commit getItem(int pos);

    void setList(QList<cvs::Commit> data);
    void appendList(QList<cvs::Commit> data);
    void updateFlags(QList<cvs::Commit> data);
    //void updateFlags(int top,int bottom);
    //void removeFlags(QList<cvs::Commit> data);

    QList<cvs::Commit> compareRows();
    int compareRow(int row);
    void clearCompareRows();



private:
    QList<cvs::Commit> m_data;
    QList<int> m_comapreRows;

};

}

#endif // COMMITMODEL_H
