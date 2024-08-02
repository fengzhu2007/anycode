#ifndef OSSPANEL_H
#define OSSPANEL_H
#include "OSS_global.h"
#include "interface/Panel.h"
#include "storage/site_storage.h"
#include <QMutex>
#include <QMimeData>
#include <QLineEdit>
namespace Ui {
class OSSPanel;
}
namespace ady {
    class OSS;
    class OSSWorker;
    class NetworkResponse;
    class OSSSetting;
    class OSS_EXPORT OSSPanel : public Panel
    {
        Q_OBJECT
    public:
        OSSPanel(long long id,QWidget* parent);
        virtual ~OSSPanel();
        virtual void init(SiteRecord record);
        virtual NetworkRequest* request();

        virtual void deleteFiles(QList<FileItem>items);
        //virtual void chmodFiles(QList<FileItem>items,int mode,bool applyChildren);
        //virtual QList<Task*> matchFilePath(QList<FileItem> items,char cmd);

    public slots:
        void workTaskFinished(QString name,NetworkResponse* response);

        //thread run functions start
        /*void link();
        void chDir(QString dir);
        void unlink();*/

        //thread run functions end

        //tree view slots start
        void onItemActivated(const QModelIndex &index);

        //tree view slots end

        void onDropUpload(const QMimeData* data);

        void onContextMenu(const QPoint& pos);

        //menu actions slots
        void onActDownload();
        //void onActChmod();
        void onActRename();
        void onActDelete();
        void onActRefresh();
        void onActMkdir();
        void onRename(const QModelIndex& index,QString newString);



    /*signals:
        void sig_link(FTP*);
        void sig_chDir(FTP*,QString);
        void sig_unlink(FTP*);*/

    private:
        Ui::OSSPanel *ui;
        OSS* oss;
        OSSWorker* worker;
        QMutex mute;
        QLineEdit* dirRender;
        OSSSetting* m_setting;
    };
}
#endif // FTPPANEL_H
