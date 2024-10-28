#include "addon_loader.h"
#include "interface/panel.h"
#include "interface/form_panel.h"
#include "network/network_request.h"
#include "network/network_manager.h"
#include "storage/addon_storage.h"
#include <QStandardPaths>
#include <QCoreApplication>
#include <QMessageBox>
#include <QTranslator>
#include <QDebug>
typedef ady::Panel*(*GET_PANEL_FUN) (long long ,QWidget*,QString);
typedef size_t (*GET_FORMPANELSIZE_FUN) (QString);
typedef ady::FormPanel*(*GET_FORMPANEL_FUN) (QWidget*,QString,size_t);
typedef int (*REQUEST_CONNECT_FUN) (void*);
typedef ady::NetworkRequest* (*INIT_REQUEST_FUN) (long long);

namespace ady {
    AddonLoader* AddonLoader::instance = nullptr;
    AddonLoader::AddonLoader()
    {
        this->m_current = nullptr;
    }

    AddonLoader* AddonLoader::getInstance(){
        if(instance==nullptr){
            instance = new AddonLoader;
        }
        return instance;
    }


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
                return false;
            }
            this->m_current = lib;
            m_loadLists[path] = lib;
        }else{
            this->m_current = m_loadLists[path];
        }
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

    Panel* AddonLoader::getPanel(long long id,QWidget* parent,const QString& name)
    {
        if(this->m_current==nullptr){
            return nullptr;
        }
        GET_PANEL_FUN fun = (GET_PANEL_FUN)this->m_current->resolve("getPanel");
        if(fun){
            return fun(id,parent,name);
        }else{
            return nullptr;
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



}
