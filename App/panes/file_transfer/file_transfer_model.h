#ifndef FILETRANSFERMODEL_H
#define FILETRANSFERMODEL_H
#include "global.h"
#include "core/event_bus/event_data.h"
#include "interface/file_item.h"
#include <QAbstractItemModel>
#include <QFile>
#include <QFileInfo>
namespace ady{





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
    FileTransferModelItem* take(int i);
    QList<FileTransferModelItem*> take(int from,int end);
    Type type();
    Mode mode();
    long long id();
    QString name() const;
    QString source() const;
    QString destination() const;
    void setName(const QString& name);
    void setSource(const QString& source);
    void setDestination(const QString& destination);
    State state();
    void setState(State state);
    float progress();
    void setProgress(float progress);
    QString errorMsg();
    void setErrorMsg(const QString& errormsg);
    bool matchedPath();
    void setMatchedPath(bool matched);
    void setFilesize(long long filesize);
    long long filesize();

public:
    static int seq;
private:
    FileTransferModelItemPrivate* d;

};









class SiteRecord;
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
    enum State{
        Stop=0,
        Running,
        Destory
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
    void appendItems(FileTransferModelItem* parent,QList<FileTransferModelItem*> items);

    void insertFrontAndRemove(long long siteid,long long id,QFileInfoList list,FileTransferModelItem::State state);
    void insertFrontAndRemove(long long siteid,long long id,QList<FileItem> list,FileTransferModelItem::State state);



    void removeItem(FileTransferModelItem* item);
    void removeItem(long long siteid,long long id);
    void removeProject(long long id);
    void removeSite(long long id);

    void removeAllItems(FileTransferModelItem::State=FileTransferModelItem::Any);
    void removeItems(QModelIndexList list,FileTransferModelItem::State=FileTransferModelItem::Any);

    void retryItems(QModelIndexList list);
    void retryAllItems();




    void openProject(long long id,const QString& name,const QString& path);

    void addSite(const SiteRecord& site);
    void addJob(UploadData* data);
    void addUploadJob(QJsonObject data);
    void addDownloadJob(QJsonObject data);

    void updateSite(const SiteRecord& site);

    void progress(long long siteid,long long id,float progress);
    void setItemFailed(long long siteid,long long id,const QString& msg);




    void start(int num);
    void stop();
    void finish();//finish all thread
    void abortJob(long long siteid);

    FileTransferModelItem* take(int siteid);
    FileTransferModelItem* rootItem();

    FileTransferModelItem* find(long long id,bool project=false);//find server

signals:
    void notifyProgress(long long siteid,long long id,float progress);

public slots:
    void onThreadFinished();
    void onUploadFolder(long long siteid,long long id,QFileInfoList list,int state);
    void onDonwloadFolder(long long siteid,long long id,QList<FileItem> list,int state);
    void onFinishTask(long long siteid,long long id);
    void onErrorTask(long long siteid,long long id,const QString& errorMsg);
    void onUpdateProgress(long long siteid,long long id,float progress);

private:
    explicit FileTransferModel(QObject *parent = nullptr);
    FileTransferModelItem* get(int siteid);
    void insertFrontItems(FileTransferModelItem* parent,QList<FileTransferModelItem*> items,FileTransferModelItem::State state);
    void remoteItems(FileTransferModelItem* site,int from,int end);

private:
    static FileTransferModel* instance;
    FileTransferModelPrivate* d;
};
}
#endif // FILETRANSFERMODEL_H
