#ifndef FILETRANSFERMODEL_H
#define FILETRANSFERMODEL_H
#include "global.h"
#include "core/event_bus/event_data.h"
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
        Job,//file
        JobGroup,//folder
    };
    enum Mode{
        None=0,
        Upload,
        Download
    };
    enum State{
        Any=0,
        Failed,
        Doing,
        Wait,
    };

    FileTransferModelItem();
    FileTransferModelItem(Type type,long long id,const QString& name,const QString& path,FileTransferModelItem* parent);//project site
    FileTransferModelItem(Mode mode,bool is_file,const QString& src,const QString& dest,FileTransferModelItem* parent);//job
    ~FileTransferModelItem();
    int childrenCount();
    int row();
    void appendItems(QList<FileTransferModelItem*> items);
    void insertItems(int position,QList<FileTransferModelItem*> items);
    void appendItem(FileTransferModelItem* item);

    FileTransferModelItem* parent();
    FileTransferModelItem* childAt(int i);
    FileTransferModelItem* findByProjectId(long long id);
    FileTransferModelItem* findBySiteId(long long id);
    FileTransferModelItem* first(State state);


    //remove
    FileTransferModelItem* take(int position);
    FileTransferJob* data();
    Type type();
    Mode mode();
    long long id();
    QString name() const;
    QString source() const;
    QString destination() const;
    State state();
    void setState(State state);

    unsigned long long filesize();

public:
    static int seq;
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
    static FileTransferModel* getInstance();
    static void destory();

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
    void insertFrontItems(FileTransferModelItem* parent,QList<FileTransferModelItem*> items,FileTransferModelItem::State state);
    void removeItem(FileTransferModelItem* item);
    void removeItem(long long siteid,long long id);

    void openProject(long long id,const QString& name,const QString& path);

    void addJob(UploadData* data);


    void start(int num);

    FileTransferModelItem* take(int siteid);


    //void addJob(long long pid,long long sid,const QString source,const QString dest);
private:
    explicit FileTransferModel(QObject *parent = nullptr);
    FileTransferModelItem* get(int siteid);

private:
    static FileTransferModel* instance;
    FileTransferModelPrivate* d;
};
}
#endif // FILETRANSFERMODEL_H
