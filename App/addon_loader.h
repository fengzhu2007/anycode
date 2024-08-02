#ifndef ADDON_LOADER_H
#define ADDON_LOADER_H
#include <QString>
#include <QMap>
#include <QLibrary>
namespace ady {
    class Panel;
    class FormPanel;
    class NetworkRequest;
    class AddonLoader
    {
    private:
        AddonLoader();

    public:
        enum AddonName{
            FTP=0,
            OSS,
            COS,
            SFTP
        };
        static AddonLoader* getInstance();
        bool load(QString file);
        bool load(AddonName name);
        Panel* getPanel(long long id,QWidget* parent,QString name);
        size_t getFormPanelSize(QString name);
        FormPanel* getFormPanel(QWidget* parent,QString name,size_t n);
        int requestConnect(void* ptr);
        NetworkRequest* initRequest(long long id);
        static void destory();


    private:
        static AddonLoader* instance;


    protected:
        QMap<QString,QLibrary*> m_loadLists;
        QLibrary* m_current;

    };
}
#endif // ADDON_LOADER_H
