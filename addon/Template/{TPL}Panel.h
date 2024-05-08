#ifndef {TPL}PANEL_H
#define {TPL}PANEL_H
#include "{TPL}_global.h"
#include "interface/Panel.h"
#include "storage/SiteStorage.h"
#include <QMutex>
#include <QMimeData>
#include <QLineEdit>
namespace Ui {
class {TPL}Panel;
}
namespace ady {
    class {TPL};
    class {TPL}Worker;
    class NetworkResponse;
    class {TPL}Setting;
    class {TPL}_EXPORT {TPL}Panel : public Panel
    {
        Q_OBJECT
    public:
        {TPL}Panel(long long id,QWidget* parent);
        virtual ~{TPL}Panel();
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
        void sig_link({TPL}*);
        void sig_chDir({TPL}*,QString);
        void sig_unlink({TPL}*);*/

    private:
        Ui::{TPL}Panel *ui;
        {TPL}* {tpl};
        {TPL}Worker* worker;
        QMutex mute;
        QLineEdit* dirRender;
        {TPL}Setting* m_setting;
    };
}
#endif // {TPL}PANEL_H
