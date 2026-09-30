#ifndef ADDON_LOADER_H
#define ADDON_LOADER_H
#include <QString>
#include <QMap>
#include <QList>
#include <QLibrary>
#include <QJsonObject>
#include <QAction>
#include "global.h"
#include "storage/addon_storage.h"

class QTextDocument;
class QMenu;

namespace TextEditor{
class LanguageLoader;
}




namespace ady {
    class FormPanel;
    class NetworkRequest;
    class DockingPaneManager;
    class DockingPane;

    struct MenuData{
        int menu_kind;
        int position;
        enum Kind { Action, Menu,Separator } kind;
        union {
            QAction* action;
            QMenu* menu;
        } ptr;
    };

    class  AddonLoader
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
        enum ExportType{
            Default = 1,
            RemoteConnector=Default,//remote connector like ftp,sftp
            OptionCategory = Default<<1,//option tab
            CodeLint = Default<<2,
            CodeAutoComplate = Default<<3,
            Pane = Default<<4,
            Editor = Default<<5,
            Menu = Default<<6,
        };
        enum OptionCategory{

        };

        enum MenuKind{
            None=0,
            File,
            Edit,
            View,
            Tool,
            Extend,
            Help,

            //sub menu
            File_New,
            File_Open
        };

        enum  MenuPosition{
            Before=999,
            After=1000
        };




        static AddonLoader* getInstance();
        void init();
        QList<AddonRecord> filter(ExportType type);
        bool loadFile(const QString& file);
        bool load(AddonName name);
        bool load(const QString name);

        size_t getFormPanelSize(const QString& name);
        FormPanel* getFormPanel(QWidget* parent,const QString& name,size_t n);
        int requestConnect(void* ptr);
        NetworkRequest* initRequest(long long id);



        bool install();
        bool uninstall();
        TextEditor::LanguageLoader* createLanguageLoader(const QString& languageName, QTextDocument* doc);



        DockingPane* makePane(DockingPaneManager* dockingManager,const QString& group,const QJsonObject& data);

        QList<MenuData> getMenus(QWidget* parent);





        static void destory();


    private:
        static AddonLoader* instance;


    protected:
        QList<AddonRecord> m_addons;
        QMap<QString,QLibrary*> m_loadLists;
        QMap<QString,QString> m_nameList;
        QMap<QString,void*> m_langLoaderFuns;
        QMap<QString,void*> m_paneFuns;
        QLibrary* m_current;

    };
}
#endif // ADDON_LOADER_H
