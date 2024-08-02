#ifndef DIFF_FILE_MODEL_H
#define DIFF_FILE_MODEL_H
#include <QAbstractTableModel>
#include "diff_file.h"
#include <QIcon>
namespace ady {

class DiffFileModelPrivate;
class DiffFileModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Column {
        Name=0,
        //Filesize,
        ModifyTime,
        Max
    };
    DiffFileModel(QObject* parent=nullptr);
    virtual ~DiffFileModel();

    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,int role = Qt::DisplayRole) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    cvs::DiffFile getItem(int pos);
    cvs::DiffFile at(int row);
    void setDataSource(QList<cvs::DiffFile> data);
    void setList(QList<cvs::DiffFile> data);
    void clear();

    void setProjectDir(QString dir);
    QList<cvs::DiffFile> lists();

private:
    DiffFileModelPrivate* d;



};

}
#endif // DIFF_FILE_MODEL_H
