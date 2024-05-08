#ifndef DIFFFILEMODEL_H
#define DIFFFILEMODEL_H
#include <QAbstractTableModel>
#include "DiffFile.h"
#include <QIcon>
namespace ady {

class DiffFileModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Column {
        Name=0,
        Filesize,
        ModifyTime,
        Max
    };
    DiffFileModel(QObject* parent=nullptr);

    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,int role = Qt::DisplayRole) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    inline void setProjectDir(QString dir){m_projectDir = dir;}

    cvs::DiffFile getItem(int pos);

    void setList(QList<cvs::DiffFile> data);
    void clear();

    inline QList<cvs::DiffFile> lists(){return m_data;}

private:
    QList<cvs::DiffFile> m_data;
    QString m_projectDir;
    QIcon m_additionIcon;
    QIcon m_deletionIcon;
    QIcon m_changeIcon;


};

}
#endif // DIFFFILEMODEL_H
