#ifndef FTPPANEL_H
#define FTPPANEL_H
#include "FTP_global.h"
#include "interface/Panel.h"
#include "storage/SiteStorage.h"
#include <QMutex>
#include <QMimeData>
#include <QLineEdit>
namespace Ui {
class FTPPanel;
}
namespace ady {
    class FTP;
    class FTPWorker;
    class NetworkResponse;
    class FTPSetting;
    class FTP_EXPORT FTPPanel : public Panel
    {
        Q_OBJECT
    public:
        FTPPanel(long long id,QWidget* parent);
        virtual ~FTPPanel();
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
        void sig_link(FTP*);
        void sig_chDir(FTP*,QString);
        void sig_unlink(FTP*);*/

    private:
        Ui::FTPPanel *ui;
        FTP* ftp;
        FTPWorker* worker;
        QMutex mute;
        QLineEdit* dirRender;
        FTPSetting* m_setting;
    };
}
#endif // FTPPANEL_H
