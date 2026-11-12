#include "addon_loader.h"
#include "interface/form_panel.h"
#include "network/network_request.h"
#include "network/network_manager.h"
#include "storage/addon_storage.h"
#include "addon_loader_helper.h"
#include <languages/loader.h>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QMessageBox>
#include <QTranslator>
#include <QDebug>
#include <QTextDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonDocument>
#include <QStringList>
#include <QFileInfo>
#include <QRegularExpression>


typedef size_t (*GET_FORMPANELSIZE_FUN) (QString);
typedef ady::FormPanel*(*GET_FORMPANEL_FUN) (QWidget*,QString,size_t);
typedef int (*REQUEST_CONNECT_FUN) (void*);
typedef ady::NetworkRequest* (*INIT_REQUEST_FUN) (long long);
typedef const char* (*ADDON_INFO_FUN) ();
typedef bool (*ADDON_UNINSTALL_FUN) ();
typedef TextEditor::LanguageLoader* (*CREATE_LANGUAGE_LOADER_FUN) (QString, QTextDocument*);
typedef ady::DockingPane* (*MAKE_PANE_FUN) (ady::DockingPaneManager*, QString, QJsonObject);
typedef QList<ady::MenuData> (*GET_MENUS_FUN) (QWidget*);
typedef GET_MENUS_FUN (*GET_GET_MENUS_FUN) ();

namespace {

    //known export type bits:Default,OptionCategory,CodeLint,CodeAutoComplate,Pane,Editor,Menu
    constexpr long long EXPORT_TYPE_MASK = 0x7f;

    struct ExportTypeName{
        const char* name;
        long long value;
    };

    constexpr ExportTypeName EXPORT_TYPE_NAMES[] = {
        {"default",1},{"remoteconnector",1},{"remote",1},
        {"optioncategory",2},{"option",2},
        {"codelint",4},{"lint",4},
        {"codeautocomplate",8},{"codeautocomplete",8},{"autocomplate",8},{"autocomplete",8},
        {"pane",16},{"editor",32},{"menu",64}
    };

    QString clampText(QString text,int maxLength = 255)
    {
        text = text.trimmed();
        if(text.size() > maxLength){
            text = text.left(maxLength);
        }
        return text;
    }

    //repair common json defects like utf-8 bom and trailing commas
    QByteArray sanitizeJsonText(const QByteArray& raw)
    {
        QByteArray data = raw;
        if(data.startsWith("\xEF\xBB\xBF")){
            data.remove(0,3);
        }
        data = data.trimmed();
        if(!data.isEmpty()){
            static const QRegularExpression trailingComma(QStringLiteral(",\\s*([}\\]])"));
            QString text = QString::fromUtf8(data);
            text.replace(trailingComma,QStringLiteral("\\1"));
            data = text.toUtf8();
        }
        return data;
    }

    QJsonValue jsonValue(const QJsonObject& json,const QStringList& keys)
    {
        for(const auto& key : keys){
            if(!json.contains(key)){
                continue;
            }
            const QJsonValue value = json.value(key);
            if(!value.isUndefined() && !value.isNull()){
                return value;
            }
        }
        return QJsonValue();
    }

    QString jsonText(const QJsonObject& json,const QStringList& keys)
    {
        const QJsonValue value = jsonValue(json,keys);
        if(value.isString()){
            return value.toString().trimmed();
        }else if(value.isDouble()){
            return QString::number(value.toDouble(),'g',10);
        }
        return QString();
    }

    long long exportTypeOne(const QJsonValue& value)
    {
        if(value.isDouble()){
            return static_cast<long long>(value.toDouble());
        }else if(!value.isString()){
            return 0;
        }
        const QString text = value.toString().trimmed();
        if(text.isEmpty()){
            return 0;
        }
        bool ok = false;
        const qlonglong number = text.toLongLong(&ok);
        if(ok){
            return number;
        }
        long long type = 0;
        QString text2 = text;
        text2.replace(QRegularExpression(QStringLiteral("[,;|\\s]+")),QStringLiteral(" "));
        const auto parts = text2.split(QLatin1Char(' '));
        for(const auto& part : parts){
            if(part.isEmpty()){
                continue;
            }
            for(const auto& item : EXPORT_TYPE_NAMES){
                if(part.compare(QLatin1String(item.name),Qt::CaseInsensitive)==0){
                    type |= item.value;
                    break;
                }
            }
        }
        return type;
    }

    long long exportTypeFromValue(const QJsonValue& value)
    {
        long long type = 0;
        if(value.isArray()){
            const QJsonArray array = value.toArray();
            for(const auto& item : array){
                type |= exportTypeOne(item);
            }
        }else{
            type = exportTypeOne(value);
        }
        return type & EXPORT_TYPE_MASK;
    }

    //keep the library path relative and free of library suffix
    QString normalizeAddonFile(QString file,const QString& loaded)
    {
        file = file.trimmed();
        file.replace(QLatin1Char('\\'),QLatin1Char('/'));
        while(file.startsWith("./")){
            file.remove(0,2);
        }
        if(file.startsWith(QLatin1Char('/'))){
            file = file.section(QLatin1Char('/'),-2);
        }
        static const QStringList suffixes = {".dll",".so",".dylib"};
        for(const auto& suffix : suffixes){
            if(file.endsWith(suffix,Qt::CaseInsensitive)){
                file.chop(suffix.size());
                break;
            }
        }
        file = file.trimmed();
        if(file.isEmpty() || file.contains("..")){
            return loaded;
        }
        return file;
    }


    ady::AddonRecord parseAddonRecord(const QByteArray& data,const QString& file)
    {
        const QJsonObject json = ady::AddonLoader::parseAddonJson(data);

        ady::AddonRecord record;
        record.id = 0;
        record.file = normalizeAddonFile(jsonText(json,{"file","library","entry","lib"}),file);
        record.name = clampText(jsonText(json,{"name","typename","type_name","typeName","id"}));
        if(record.name.isEmpty() && !record.file.isEmpty()){
            //fallback to the library directory name
            record.name = record.file.section(QLatin1Char('/'),0,0);
        }
        record.title = clampText(jsonText(json,{"title","displayName","label","name"}));
        if(record.title.isEmpty()){
            record.title = record.name;
        }
        record.label = clampText(jsonText(json,{"label","title","name"}));
        if(record.label.isEmpty()){
            record.label = record.title;
        }
        record.version = clampText(jsonText(json,{"version","ver"}),32);
        if(record.version.isEmpty()){
            record.version = QStringLiteral("1.0");
        }
        record.author = clampText(jsonText(json,{"author","publisher","company"}));
        if(record.author.isEmpty()){
            record.author = QStringLiteral("Unknown");
        }
        record.description = clampText(jsonText(json,{"description","desc","summary"}),1024);
        record.export_type = exportTypeFromValue(jsonValue(json,{"export_type","exportType","export","type"}));
        record.status = 1;
        record.is_system = 0;
        return record;
    }
}





namespace ady {

    AddonLoader* AddonLoader::instance = nullptr;
    AddonLoader::AddonLoader():
        d(new AddonLoaderPrivate)
    {
        this->m_current = nullptr;
    }

    AddonLoader::~AddonLoader(){
        delete d;
    }


    AddonLoader* AddonLoader::getInstance(){
        if(instance==nullptr){
            instance = new AddonLoader;
        }
        return instance;
    }

    void AddonLoader::init(){
        AddonStorage storage;
        d->m_addons = storage.list(1);
    }

    /*QList<AddonRecord> AddonLoader::filter(ExportType type){
        QList<AddonRecord> result;
        int flag = static_cast<int>(type);
        for(const auto& record : d->m_addons){
            if((record.export_type & flag) == flag){
                result.append(record);
            }
        }
        return result;
    }*/


    bool AddonLoader::loadFile(const QString& file)
    {

      //QString dir = QCoreApplication::applicationDirPath();
       QString appPath = QCoreApplication::applicationDirPath();
       QStringList arr = file.split("/");
#ifdef Q_OS_MAC
        //qDebug()<<"file:"<<file;
        ///Users/wangjian/Desktop/Projects/ady/build/addon/FTP



        QString path = QString("%1/addon/%2/%3").arg(appPath).arg(arr[0]).arg(arr[1]);
        //qDebug()<<"path:"<<path;
        //QMessageBox::information(nullptr,"11",path);
#else
    QString path = QString("addon/%1").arg(file);




#endif

        QTranslator* translator = new QTranslator();
        if(translator->load(QString("%1/addon/%2/%3").arg(appPath).arg(arr[0]).arg("language.zh_CN.qm"))){
            QCoreApplication::installTranslator(translator);
        }else{
            delete translator;
        }
        //qDebug()<<"path:"<<path;
        if(this->m_loadLists.find(path)==m_loadLists.end()){
            QLibrary * lib = new QLibrary(path);
            bool ret = lib->load();
            //qDebug()<<"file:"<<file<<";result:"<<ret;
            if(ret==false){
                delete lib;
                return false;
            }
            this->m_current = lib;
            m_loadLists[path] = lib;
        }else{
            this->m_current = m_loadLists[path];
        }
        this->m_currentFile = file;
        return true;
    }

    bool AddonLoader::load(AddonName name){
        QString file;
        if(name==FTP){
            file = QString::fromUtf8("FTP/FTP");
        }else if(name==OSS){
            file = QString::fromUtf8("OSS/OSS");
        }else if(name==COS){
            file = QString::fromUtf8("COS/COS");
        }else if(name==SFTP){
            file = QString::fromUtf8("SFTP/SFTP");
        }else if(name==S3){
            file = QString::fromUtf8("S3/S3");
        }else{
            return false;
        }
        return this->loadFile(file);
    }

    bool AddonLoader::load(const QString name){
        if(m_nameList.contains(name)){
            return loadFile(m_nameList[name]);
        }else{
            //load
            AddonStorage storage;
            auto r = storage.one(name);
            if(r.id>0){
                m_nameList.insert(r.name,r.file);
                return loadFile(r.file);
            }else{
                return false;
            }
        }
    }



    size_t AddonLoader::getFormPanelSize(const QString& name)
    {
        if(this->m_current==nullptr){
            return 0;
        }
        GET_FORMPANELSIZE_FUN fun = (GET_FORMPANELSIZE_FUN)this->m_current->resolve("getFormPanelSize");
        if(fun){
            return fun(name);
        }else{
            return 0;
        }
    }

    FormPanel* AddonLoader::getFormPanel(QWidget* parent,const QString& name,size_t n)
    {
        if(this->m_current==nullptr){
            return nullptr;
        }
        GET_FORMPANEL_FUN fun = (GET_FORMPANEL_FUN)this->m_current->resolve("getFormPanel");
        if(fun){
            return fun(parent,name,n);
        }else{
            return nullptr;
        }
    }

    int AddonLoader::requestConnect(void* ptr)
    {
        if(this->m_current==nullptr){
            return -2;
        }
        REQUEST_CONNECT_FUN fun = (REQUEST_CONNECT_FUN)this->m_current->resolve("requestConnect");
        if(fun){
            return fun(ptr);
        }else{
            return -2;
        }
    }

    NetworkRequest* AddonLoader::initRequest(long long id){
        if(this->m_current==nullptr){
            return nullptr;
        }
        INIT_REQUEST_FUN fun = (INIT_REQUEST_FUN)this->m_current->resolve("initRequest");
        if(fun){
            return fun(id);
        }else{
            return nullptr;
        }
    }

    void AddonLoader::destory(){
        if(instance!=nullptr){
            delete instance;
            instance = nullptr;
        }
    }

    bool AddonLoader::install()
    {
        if (this->m_current == nullptr)
            return false;

        //the addon only provides its metadata,the database is maintained here
        QByteArray metadata;
        ADDON_INFO_FUN fun = (ADDON_INFO_FUN)this->m_current->resolve("addonInfo");
        if(fun!=nullptr){
            const char* text = fun();
            if(text!=nullptr){
                metadata = QByteArray(text);
            }
        }
        if(metadata.isEmpty()){
            qWarning() << "addon:" << m_currentFile << "does not provide metadata,use the file entry instead";
        }

        AddonRecord record = parseAddonRecord(metadata,m_currentFile);
        if(record.name.isEmpty()){
            qWarning() << "addon: metadata has no valid name,file:" << m_currentFile;
            return false;
        }
        if(record.file.isEmpty()){
            record.file = m_currentFile;
        }else if(!m_currentFile.isEmpty() && record.file!=m_currentFile){
            //the loaded entry wins when the declared library does not exist
            const QString base = QCoreApplication::applicationDirPath() + "/addon/" + record.file;
            static const QStringList suffixes = {QString(),QStringLiteral(".dll"),QStringLiteral(".so"),QStringLiteral(".dylib")};
            bool exists = false;
            for(const auto& suffix : suffixes){
                if(QFileInfo::exists(base + suffix)){
                    exists = true;
                    break;
                }
            }
            if(!exists){
                record.file = m_currentFile;
            }
        }
        if(record.export_type<=0){
            record.export_type = guessExportType();
            if(record.export_type<=0){
                record.export_type = Default;
            }
        }
        record.status = 1;

        AddonStorage storage;
        AddonRecord exist = storage.one(record.name);
        bool ret = false;
        if(exist.id>0){
            //keep system flag and primary key,refresh the other fields
            record.id = exist.id;
            record.is_system = exist.is_system;
            ret = storage.update(record);
        }else{
            record.is_system = 0;
            ret = storage.insert(record)>0;
        }
        if(ret){
            m_nameList[record.name] = record.file;
            this->init();
        }else{
            qWarning() << "addon: save record failed:" << storage.errorText() << record.name;
        }
        return ret;
    }

    bool AddonLoader::install(const QString& file)
    {
        return this->loadFile(file) && this->install();
    }

    bool AddonLoader::uninstall()
    {
        if(this->m_currentFile.isEmpty()){
            return false;
        }
        AddonStorage storage;
        const auto records = storage.all();
        for(const auto& record : records){
            if(record.file==m_currentFile){
                return this->uninstall(record.name);
            }
        }
        return false;
    }

    bool AddonLoader::uninstall(const QString& name)
    {
        const QString addonName = name.trimmed();
        if(addonName.isEmpty()){
            return false;
        }
        AddonStorage storage;
        AddonRecord record = storage.one(addonName);
        if(record.id<=0 || record.is_system){
            return false;
        }
        //let the addon clean its own runtime resources,the record is removed here
        const QString file = m_nameList.value(addonName,record.file);
        if(!file.isEmpty() && this->loadFile(file)){
            ADDON_UNINSTALL_FUN fun = (ADDON_UNINSTALL_FUN)this->m_current->resolve("uninstall");
            if(fun!=nullptr){
                fun();
            }
        }
        const bool ret = storage.del(record.id);
        if(ret){
            m_nameList.remove(addonName);
            this->init();
        }else{
            qWarning() << "addon: remove record failed:" << storage.errorText() << addonName;
        }
        return ret;
    }

    QJsonObject AddonLoader::parseAddonJson(const QByteArray& data)
    {
        const QByteArray text = sanitizeJsonText(data);
        if(text.isEmpty()){
            return QJsonObject();
        }
        QJsonParseError error;
        const QJsonDocument document = QJsonDocument::fromJson(text,&error);
        if(error.error!=QJsonParseError::NoError || !document.isObject()){
            qWarning() << "addon: invalid metadata json:" << error.errorString() << text;
            return QJsonObject();
        }
        return document.object();
    }



    long long AddonLoader::guessExportType()
    {
        if(this->m_current==nullptr){
            return 0;
        }
        long long type = 0;
        if(this->m_current->resolve("makePane")!=nullptr){
            type |= Pane;
        }
        if(this->m_current->resolve("getGetMenus")!=nullptr){
            type |= Menu;
        }
        if(this->m_current->resolve("createLanguageLoader")!=nullptr){
            type |= CodeAutoComplate;
        }
        if(this->m_current->resolve("requestConnect")!=nullptr || this->m_current->resolve("getFormPanel")!=nullptr){
            type |= RemoteConnector;
        }
        return type;
    }

    TextEditor::LanguageLoader* AddonLoader::createLanguageLoader(const QString& languageName, QTextDocument* doc)
    {
        //check cache by languageName
        auto it = m_langLoaderFuns.find(languageName);
        if(it!=m_langLoaderFuns.end()){
            CREATE_LANGUAGE_LOADER_FUN fun = (CREATE_LANGUAGE_LOADER_FUN)it.value();
            return fun(languageName, doc);
        }

        //search installed CodeAutoComplate addons
        auto addons = d->filter(CodeAutoComplate);
        for(const auto& addon : addons){
            if(addon.name.contains(languageName, Qt::CaseInsensitive)){
                if(loadFile(addon.file)){
                    CREATE_LANGUAGE_LOADER_FUN fun = (CREATE_LANGUAGE_LOADER_FUN)m_current->resolve("createLanguageLoader");
                    if(fun){
                        m_langLoaderFuns[languageName] = (void*)fun;
                        return fun(languageName, doc);
                    }
                }
            }
        }
        return nullptr;
    }

    DockingPane* AddonLoader::makePane(DockingPaneManager* dockingManager, const QString& group, const QJsonObject& data)
    {
        //check cache by group
        auto it = m_paneFuns.find(group);
        if(it!=m_paneFuns.end()){
            MAKE_PANE_FUN fun = (MAKE_PANE_FUN)it.value();
            return fun(dockingManager, group, data);
        }

        //search installed Pane addons
        auto addons = d->filter(Pane);
        for(const auto& addon : addons){
            if(addon.name==group){
                if(loadFile(addon.file)){
                    MAKE_PANE_FUN fun = (MAKE_PANE_FUN)m_current->resolve("makePane");
                    if(fun){
                        m_paneFuns[group] = (void*)fun;
                        return fun(dockingManager, group, data);
                    }
                }
            }
        }
        return nullptr;
    }

    QList<MenuData> AddonLoader::getMenus(QWidget* parent)
    {
        QList<MenuData> result;
        //search installed Menu addons
        auto addons = d->filter(Menu);
        for(const auto& addon : addons){
            if(loadFile(addon.file)){
                GET_GET_MENUS_FUN getter = (GET_GET_MENUS_FUN)m_current->resolve("getGetMenus");
                if(getter){
                    GET_MENUS_FUN fun = getter();
                    if(fun){
                        result.append(fun(parent));
                    }
                }
            }
        }
        return result;
    }

}
