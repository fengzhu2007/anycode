#ifndef PANEL_H
#define PANEL_H
#include "global.h"
#include <QWidget>
#include <QList>
#include "storage/SiteStorage.h"
#include "local/FileItem.h"
#include "NotificationImpl.h"
namespace ady {
    class NetworkRequest;
    class Task;
    class ANYENGINE_EXPORT Panel : public QWidget , public NotificationImpl
    {
        Q_OBJECT
    public:
        enum CMD {
            UPLOAD=0,
            DOWNLOAD,
            DEL,
            CHMOD,
        };


        Panel(long long id,QWidget* parent);
        virtual ~Panel();


        void setId(long long id);
        void setGroupid(long long gid);
        void setType(QString t);

        long long getId();
        long long getGroupid();
        QString getType();
        void setPorjectDir(QString dir);

        virtual NetworkRequest* request();
        virtual void init(SiteRecord record);
        //virtual QString matchFilePath();
        virtual QList<Task*> matchFilePath(QList<FileItem> items,char cmd);
        virtual QString dirSync(const QString& filename,char cmd,bool& sync);
        virtual QString dirMapping(const QString& filename,char cmd);
        virtual bool filterExtension(const QString name,char cmd);
        virtual bool filterDir(const QString name,char cmd);

        virtual void deleteFiles(QList<FileItem>items);
        virtual void chmodFiles(QList<FileItem>items,int mode,bool applyChildren);


    protected:
        virtual void resizeEvent(QResizeEvent *event) override;



    public slots:


    signals:
        void command(CMD cmd,long long id,QList<FileItem> items,QString dstDir,bool keepDir);
        void command(QList<Task*> tasks);


    protected:
        long long id;
        long long groupid;
        QString type;
        QString projectDir;
        SiteRecord site;
        QString currentDir;
        QList<QPair<QString,QString>> m_dirSync;
        QList<QPair<QString,QString>> m_dirMapping;
        QStringList m_filterDirs;
        QStringList m_filterExts;




    };
}
#endif // PANEL_H
