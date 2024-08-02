#ifndef SFTPPANEL_H
#define SFTPPANEL_H
#include "SFTP_global.h"
#include "interface/Panel.h"
#include "storage/site_storage.h"
#include <QMutex>
#include <QMimeData>
#include <QLineEdit>
namespace Ui {
class SFTPPanel;
}
namespace ady {
    class SFTP;
    class SFTPWorker;
    class NetworkResponse;
    class SFTPSetting;
    class SFTP_EXPORT SFTPPanel : public Panel
    {
        Q_OBJECT
    public:
        SFTPPanel(long long id,QWidget* parent);
        virtual ~SFTPPanel();
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
        void onActChmod();
        void onActRename();
        void onActDelete();
        void onActRefresh();
        void onActMkdir();
        void onRename(const QModelIndex& index,QString newString);



    /*signals:
        void sig_link(SFTP*);
        void sig_chDir(SFTP*,QString);
        void sig_unlink(SFTP*);*/

    private:
        Ui::SFTPPanel *ui;
        SFTP* sftp;
        SFTPWorker* worker;
        QMutex mute;
        QLineEdit* dirRender;
        SFTPSetting* m_setting;
    };
}
#endif // SFTPPANEL_H
