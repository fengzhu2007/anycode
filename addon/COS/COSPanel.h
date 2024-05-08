#ifndef COSPANEL_H
#define COSPANEL_H
#include "COS_global.h"
#include "interface/Panel.h"
#include "storage/SiteStorage.h"
#include <QMutex>
#include <QMimeData>
#include <QLineEdit>
namespace Ui {
class COSPanel;
}
namespace ady {
    class COS;
    class COSWorker;
    class NetworkResponse;
    class COSSetting;
    class COS_EXPORT COSPanel : public Panel
    {
        Q_OBJECT
    public:
        COSPanel(long long id,QWidget* parent);
        virtual ~COSPanel();
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
        //void onActRename();
        void onActDelete();
        void onActRefresh();
        void onActMkdir();



    /*signals:
        void sig_link(COS*);
        void sig_chDir(COS*,QString);
        void sig_unlink(COS*);*/

    private:
        Ui::COSPanel *ui;
        COS* cos;
        COSWorker* worker;
        QMutex mute;
        QLineEdit* dirRender;
        COSSetting* m_setting;
    };
}
#endif // COSPANEL_H
