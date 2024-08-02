#ifndef FILETRANSFERMODEL_H
#define FILETRANSFERMODEL_H
#include "global.h"
#include <QAbstractItemModel>
#include <QFile>
namespace ady{

struct ANYENGINE_EXPORT FileTransferJob{
public:
    unsigned long long total;
    unsigned long long ready;
    QFile* file;
};




class FileTransferModelItemPrivate;
class ANYENGINE_EXPORT FileTransferModelItem{
public:
    enum Type{
        Solution=0,
        Project,
        Server,
        Job
    };
    enum Mode{
        None=0,
        Upload,
        Download
    };
    FileTransferModelItem();
    FileTransferModelItem(Type type,long long id,const QString& name,FileTransferModelItem* parent);//project site
    FileTransferModelItem(Mode mode,const QString& src,const QString& dest,FileTransferModelItem* parent);//job
    ~FileTransferModelItem();
    int childrenCount();
    int row();
    void appendItems(QList<FileTransferModelItem*> items);
    void appendItem(FileTransferModelItem* item);
    FileTransferModelItem* parent();
    FileTransferModelItem* childAt(int i);
    FileTransferJob* data();
    Type type();
    Mode mode();
    QString name() const;
    QString source() const;
    QString destination() const;
    unsigned long long filesize();

private:
    FileTransferModelItemPrivate* d;
};










class FileTransferModelPrivate;
class ANYENGINE_EXPORT FileTransferModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum Column {
        Name=0,
        FileSize,
        Src,
        Dest,
        Status,
        Max
    };
    explicit FileTransferModel(QObject *parent = nullptr);
    ~FileTransferModel();

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;



    void appendItem(FileTransferModelItem* parent,FileTransferModelItem* item);

    void openProject(long long id,const QString name);

private:
    FileTransferModelPrivate* d;
};
}
#endif // FILETRANSFERMODEL_H
