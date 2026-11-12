#ifndef ADDON_LOADER_H
#define ADDON_LOADER_H
#include <QString>
#include <QMap>
#include <QList>
#include <QByteArray>
#include <QLibrary>
#include <QJsonObject>
#include <QAction>
#include "global.h"

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
    class AddonLoaderPrivate;
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



        ~AddonLoader();
        static AddonLoader* getInstance();
        void init();

        bool loadFile(const QString& file);
        bool load(AddonName name);
        bool load(const QString name);

        size_t getFormPanelSize(const QString& name);
        FormPanel* getFormPanel(QWidget* parent,const QString& name,size_t n);
        int requestConnect(void* ptr);
        NetworkRequest* initRequest(long long id);



        /*
         * Addon metadata protocol
         *
         * An addon only exports the function below,it returns a UTF-8 json
         * string which describes the addon.All database works are done here.
         *
         *  extern "C" const char* addonInfo();
         *
         * {
         *   "name": "FTP",                 //unique name,aliases:typename/type_name/id
         *   "title": "FTP",                //display name
         *   "label": "FTP",                //short label
         *   "file": "FTP/FTP",             //library path relative to addon dir
         *   "export_type": "RemoteConnector", //int,string or string array
         *   "version": "1.0",
         *   "author": "Official",
         *   "description": "..."
         * }
         *
         * Invalid json,unknown keys,missing fields,absolute path and legacy
         * fields are normalized by parseAddonRecord() before saving.
         */
        bool install();
        bool install(const QString& file);
        bool uninstall();
        bool uninstall(const QString& name);
        TextEditor::LanguageLoader* createLanguageLoader(const QString& languageName, QTextDocument* doc);



        DockingPane* makePane(DockingPaneManager* dockingManager,const QString& group,const QJsonObject& data);

        QList<MenuData> getMenus(QWidget* parent);

        //normalize the raw metadata json,an empty object is returned when invalid
        static QJsonObject parseAddonJson(const QByteArray& data);
        //convert the metadata json to a filled and validated record,export_type is 0 when not declared


        static void destory();


    private:
        //guess export type from exported symbols when the metadata does not declare it
        long long guessExportType();

        static AddonLoader* instance;


    protected:

        QMap<QString,QLibrary*> m_loadLists;
        QMap<QString,QString> m_nameList;
        QMap<QString,void*> m_langLoaderFuns;
        QMap<QString,void*> m_paneFuns;
        QLibrary* m_current;
        QString m_currentFile;//relative addon entry of the current library,like "FTP/FTP"

    private:
        AddonLoaderPrivate* d;

    friend class AddonLoaderHelper;

    };
}
#endif // ADDON_LOADER_H
