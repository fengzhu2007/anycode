#ifndef ADDONLOADER_H
#define ADDONLOADER_H
#include <QString>
#include <QMap>
#include <QLibrary>
namespace ady {
    class Panel;
    class FormPanel;
    class AddonLoader
    {
    private:
        AddonLoader();

    public:
        static AddonLoader* getInstance();
        bool load(QString file);
        Panel* getPanel(long long id,QWidget* parent,QString name);
        size_t getFormPanelSize(QString name);
        FormPanel* getFormPanel(QWidget* parent,QString name,size_t n);
        int requestConnect(void* ptr);
        static void destory();


    private:
        static AddonLoader* instance;


    protected:
        QMap<QString,QLibrary*> m_loadLists;
        QLibrary* m_current;

    };
}
#endif // ADDONLOADER_H
