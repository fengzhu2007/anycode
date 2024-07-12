#include "Panel.h"
#include "common/utils.h"
#include "transfer/Task.h"
#include "components/Notification.h"
#include <QFileInfo>
#include <QDebug>
namespace ady {

    Panel::Panel(long long id,QWidget* parent)
        :QWidget(parent),NotificationImpl(parent)
    {
        this->id = id;
    }

    Panel::~Panel()
    {
        //NotificationImpl::~NotificationImpl();
    }

    void Panel::setId(long long id)
    {
        this->id = id;
    }

    void Panel::setGroupid(long long gid)
    {
        this->groupid = gid;
    }
    void Panel::setType(QString t)
    {
        this->type = t;
    }

    long long Panel::getId()
    {
        return this->id;
    }
    long long Panel::getGroupid()
    {
        return this->groupid;
    }
    QString Panel::getType()
    {
        return this->type;
    }

    void Panel::setPorjectDir(QString dir)
    {
        this->projectDir = dir;
    }

    NetworkRequest* Panel::request()
    {
        return nullptr;
    }

    void Panel::init(SiteRecord record)
    {
        this->site = record;
    }

    QList<Task*> Panel::matchFilePath(QList<FileItem> items,char cmd)
    {
        //cmd 0=upload 1=download 2=del 3=chmod
        QList<Task*> tasks;
        if(cmd==0){
            //items is local file lists
            //action upload
            int size = this->projectDir.size();
            QList<FileItem>::iterator iter = items.begin();
            while(iter!=items.end())
            {
                QString prefix = (*iter).path.left(size);
                if(prefix == this->projectDir){
                    QString rPath = (*iter).path.mid(size);//Relative path



                    if((*iter).type==FileItem::File){
                        bool allow = filterExtension(rPath,cmd);
                        if(!allow){
                            //continue;
                            goto to_continue;
                        }
                    }

                    QString unixPath = Utils::toUnixPath(rPath);

                    QString sitePath = this->site.path;
                    if(!sitePath.endsWith("/")){
                        sitePath += "/";
                    }

                    if(unixPath.startsWith("/")){
                        unixPath = unixPath.mid(1);
                    }

                    bool allow = filterDir(unixPath,cmd);
                    if(!allow){
                        goto to_continue;
                    }

                    bool sync = false;
                    //qDebug()<<"unixPath:"<<unixPath;
                    //qDebug()<<"data:"<<m_dirSync;
                    QString syncPath = this->dirSync(unixPath,cmd,sync);


                    QString unixMappingPath = this->dirMapping(unixPath,cmd);
                    QString absPath = sitePath + unixMappingPath;
                    Task* t = new Task();
                    t->type = (*iter).type==FileItem::File?0:1;
                    t->local = (*iter).path;
                    t->remote = absPath;
                    t->siteid = this->id;
                    t->filesize = (*iter).size;
                    t->cmd = cmd;
                    tasks.push_back(t);

                    //qDebug()<<"sync:"<<sync;

                    if(sync){

                        QString localPath = this->projectDir;
                        if(localPath.endsWith("/")==false){
                            localPath += "/";
                        }

                        if(syncPath.startsWith("/")){
                            syncPath = syncPath.mid(1);
                        }

                        QString syncMappingPath = this->dirMapping(syncPath,cmd);
                        QString absPath = sitePath + syncMappingPath;
                        localPath += syncPath;

                        if(QFileInfo::exists(localPath)){

                            Task* t = new Task();
                            t->type = (*iter).type==FileItem::File?0:1;
                            t->local = localPath;
                            t->remote = absPath;
                            t->siteid = this->id;
                            if(t->type==0){
                                QFileInfo fi(t->local);
                                t->filesize = fi.size();
                            }
                            t->cmd = cmd;
                            tasks.push_back(t);

                        }

                    }
                }

                to_continue:

                iter++;
            }
        }else if(cmd==1/* || cmd==2*/){
            QList<FileItem>::iterator iter = items.begin();
            qDebug()<<"site path:"<<this->site.path;
            QString rootPath = this->site.path;
            if(!rootPath.startsWith("/")){
                rootPath = ("/"+rootPath);
            }
            int size = rootPath.size();
            while(iter!=items.end())
            {
                QString path = (*iter).path;
                if(path.endsWith("/")){
                    path = path.left((*iter).path.length() - 1);
                }
                qDebug()<<"site path:"<<path;
                QString prefix = path.left(size);
                qDebug()<<"path:"<<path<<";path1:"<<rootPath;
                if(prefix == rootPath){
                    QString rPath = path.mid(size);//Relative path
                    //QString localPath = Utils::toLocalPath(rPath);
                    QString absPath = this->projectDir;
                    if(!absPath.endsWith("/")){
                        absPath += "/";
                    }
                    if(rPath.endsWith("/")){
                        rPath = rPath.left(rPath.length() - 1);
                    }
                    qDebug()<<"rPath:"<<rPath;
                    QString rMappingPath = this->dirMapping(rPath,cmd);
                     qDebug()<<"rPath:"<<rMappingPath;
                    //absPath += rPath;
                    absPath += rMappingPath;
                    Task* t = new Task();
                    t->type = ((*iter).type==FileItem::File)?0:1;
                    if(t->type==1 && absPath.endsWith("/")==false){
                        absPath += "/";
                    }

                    t->local = absPath;
                    t->remote = path;
                    t->siteid = this->id;
                    t->filesize = (*iter).size;
                    t->cmd = cmd;
                    tasks.push_back(t);
                }
                iter++;
            }
        }
        return tasks;
    }

    //upload sync
    QString Panel::dirSync(const QString& filename,char cmd, bool& sync)
    {
        int length = filename.length();
        if(cmd==0){
            for(auto item:m_dirSync){
                int size = item.first.length();
                if(length>=size){
                    QString prefix = filename.left(size);
                    if(prefix==item.first){
                        sync = true;
                        return item.second + filename.mid(size);
                    }
                }
            }

        }/*else if(cmd==1){
            for(auto item:m_dirSync){
                int size = item.second.length();
                if(length>=size){
                    QString prefix = filename.left(size);
                    if(prefix==item.second){
                        sync = true;
                        return item.first + filename.mid(size);
                    }
                }
            }
        }*/
        return filename;
    }

    QString Panel::dirMapping(const QString& filename,char cmd)
    {

        int length = filename.length();
        if(cmd==0){
            for(auto item:m_dirMapping){
                int size = item.first.length();
                if(length>=size){
                    /*if(item.second=="/"){
                        return filename;
                    }else{

                    }*/
                    QString prefix = filename.left(size);
                    if(prefix==item.first){
                        if(item.second=="/"){
                            return filename.mid(size);
                        }else{
                            return item.second + filename.mid(size);
                        }

                    }
                }
            }

        }else if(cmd==1){
            for(auto item:m_dirMapping){

                int size = item.second.length();
                if(length>=size){
                     QString prefix = filename.left(size);
                     if(prefix==item.second){
                        return item.first + filename.mid(size);
                     }else if(item.second=="/"){
                        return item.first + filename;
                     }
                }
             }


        }
        return filename;
    }

    bool Panel::filterExtension(const QString name,char cmd)
    {
        qDebug()<<"exts:"<<m_filterExts;
        if(cmd==0){
            //upload
            if(m_filterExts.size()>0){
                int pos = name.lastIndexOf(".");
                qDebug()<<"name:"<<name;
                if(pos>-1){
                    QString ext = name.mid(pos + 1).toLower();
                    qDebug()<<"Ext:"<<ext;
                    return m_filterExts.contains(ext);
                }else{
                    return false;
                }
            }else{
                return true;
            }
        }else{
            return true;
        }
    }

    bool Panel::filterDir(const QString name,char cmd)
    {
        if(cmd==0){
            if(m_filterDirs.size()>0){
                qDebug()<<"name:"<<name;
                int length = name.size();
                for(auto item:m_filterDirs){
                    int size = item.size();
                    qDebug()<<"item size:"<<size<<";item:"<<item;
                    if(length>=size){
                        QString prefix = name.left(size);
                        qDebug()<<"prefix:"<<prefix;
                        if(prefix==item){
                            return true;
                        }
                    }
                }
                return false;
            }else{
                return true;
            }
        }else{
            return true;
        }
    }

    void Panel::deleteFiles(QList<FileItem>items)
    {
        Q_UNUSED(items);
    }

    void Panel::chmodFiles(QList<FileItem>items,int mode,bool applyChildren)
    {
        Q_UNUSED(items);
        Q_UNUSED(mode);
        Q_UNUSED(applyChildren);
    }

    void Panel::resizeEvent(QResizeEvent *event)
    {
        if(m_notification!=nullptr){
            m_notification->resize();
        }
    }

}
