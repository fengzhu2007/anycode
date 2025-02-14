#ifndef ADDON_LOADER_H
#define ADDON_LOADER_H
#include <QString>
#include <QMap>
#include <QLibrary>
namespace ady {
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
            SFTP,
            S3
        };
        static AddonLoader* getInstance();
        bool loadFile(const QString& file);
        bool load(AddonName name);
        bool load(const QString name);

        size_t getFormPanelSize(const QString& name);
        FormPanel* getFormPanel(QWidget* parent,const QString& name,size_t n);
        int requestConnect(void* ptr);
        NetworkRequest* initRequest(long long id);
        static void destory();


    private:
        static AddonLoader* instance;


    protected:
        QMap<QString,QLibrary*> m_loadLists;
        QMap<QString,QString> m_nameList;
        QLibrary* m_current;

    };
}
#endif // ADDON_LOADER_H
