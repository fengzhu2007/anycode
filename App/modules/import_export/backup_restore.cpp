#include "backup_restore.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDir>
#include <QDateTime>

#include "common.h"
#include "core/des.h"
#include "select_tree_model.h"
#include "common/utils.h"


namespace ady{
constexpr int version = 1;
const QString key = "Anycode00";
const QString vt = "Anycode11";
const QString fkey = "data";

class BackupRestorePrivate{
public:
    QJsonObject data;
    QJsonArray array;
    QList<ProjectRecord> projects;
    QList<SiteRecord> sites;

};

BackupRestore::BackupRestore() {
    d = new BackupRestorePrivate;
    d->projects = ProjectStorage().all();
    d->sites = SiteStorage().all();
}


BackupRestore::~BackupRestore(){
    delete d;
}

void BackupRestore::appendProject(long long id,const QList<long long>& sites){
    QJsonArray sitesJson;
    for(auto sid:sites){
        auto rr = findSite(sid);
        if(rr.id>0){
            if(!rr.password.isEmpty()){
                rr.password = BackupRestore::encode(rr.password);//password encode
            }
            sitesJson<<rr.toJson();
        }
    }
    auto project = findProject(id);
    if(project.id>0){
        project.cvs_password = BackupRestore::encode(project.cvs_password);//password encode
        auto proj = project.toJson();
        proj.insert("list",sitesJson);
        QJsonObject item = {
            {"type",SelectTreeModelItem::Project},
            {"item",proj}
        };
        d->array<<item;
        /*if(d->data.contains(fkey)){
            d->data.find(fkey)->toArray().append(item);
        }else{
            QJsonArray array;
            array<<item;
            d->data.insert(fkey,array);
        }*/
    }
}

void BackupRestore::appendSite(long long id){
    auto site = this->findSite(id);
    if(site.id>0){
        site.password = BackupRestore::encode(site.password);//password encode
        QJsonObject item = {
            {"type",SelectTreeModelItem::Site},
            {"item",site.toJson()}
        };
        d->array<<item;
        /*if(d->data.contains(fkey)){
            d->data.find(fkey)->toArray().append(item);
        }else{
            QJsonArray array;
            array<<item;
            d->data.insert(fkey,array);
        }*/
    }
}

BackupRestore::Error BackupRestore::save(const QString& directory,const QString& filename){
    if(!Utils::isFilename(filename)){
        return Invalid_Filename;
    }
    //mkdir
    QDir dir;
    if(QFileInfo::exists(directory)==false && !dir.mkpath(directory)){
        return Invalid_Directory;
    }

    QFile file(directory+"/"+filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return Write_Failed;
    }
    d->data = {
            {"version",version},
            {"name",APP_NAME+" Export Data"},
            {"creator",APP_NAME+" "+APP_VERSION},
            {"date",QDateTime::currentDateTime().toString()},
    };
    d->data.insert(fkey,d->array);
    QJsonDocument doc;
    doc.setObject(d->data);
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out <<doc.toJson(QJsonDocument::Indented);
    file.close();
    return None;
}

BackupRestore::Error BackupRestore::saveAll(const QString& directory,const QString& filename){
    for(auto one:d->projects){
        QList<long long>sites;
        for(auto s:d->sites){
            if(s.pid==one.id){
                sites << s.id;
            }
        }
        this->appendProject(one.id,sites);
    }
    for(auto one:d->sites){
        if(one.pid==0){
            this->appendSite(one.id);
        }
    }
    return save(directory,filename);
}

BackupRestore::Error BackupRestore::readFile(const QString& path){
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return Invalid_Json_File;
    }
    QTextStream in(&file);
    in.setCodec("UTF-8");
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(in.readAll().toUtf8(),&error);
    if(error.error==QJsonParseError::NoError){
        d->data = doc.object();
        return None;
    }else{
        qDebug()<<"json parse error:"<<error.errorString()<<";offset:"<<error.offset;
        return Json_Exception;
    }
}

void BackupRestore::restore(long long id,const QList<long long>& sites,bool merge){
    auto array = d->data.find(fkey)->toArray();
    ProjectRecord proj;
    QList<SiteRecord>list;
    for(auto one:array){
        auto json = one.toObject();
        auto type = json.find("type")->toInt(0);
        if(type==SelectTreeModelItem::Project){
            auto item = json.find("item")->toObject();
            if(item.find("id")->toVariant().toLongLong()==id){
                proj.fromJson(item);
                auto sitelist = item.find("list")->toArray();
                for(auto s:sitelist){
                    SiteRecord ss;
                    ss.fromJson(s.toObject());
                    if(sites.contains(ss.id)){
                        list<<ss;
                    }
                }
            }
        }
    }
    if(proj.id==0){
        return ;
    }
    if(proj.cvs_password.isEmpty()==false){
        proj.cvs_password = decode(proj.cvs_password);//decode csv_password
    }
    ProjectStorage projectStorage;
    SiteStorage siteStorage;
    if(!merge){
        auto pid = projectStorage.insert(proj);
        if(pid!=0){
            for(auto one:list){
                if(one.password.isEmpty()==false){
                    one.password = decode(one.password);//decode site password
                }
                //qDebug()<<"password"<<one.password;
                one.pid = pid;
                siteStorage.insert(one);
            }
        }
    }else{
        //fine project by name
        auto r = projectStorage.one(proj.name);
        if(r.id==0){
            //insert
            auto pid = projectStorage.insert(proj);
            if(pid!=0){
                for(auto one:list){
                    if(one.password.isEmpty()==false){
                        one.password = decode(one.password);//decode site password
                    }
                    one.pid = pid;
                    siteStorage.insert(one);
                }
            }
        }else{
            //update
            for(auto one:list){
                //one.pid = pid;
                if(one.password.isEmpty()==false){
                    one.password = decode(one.password);//decode site password
                }
                auto rr = siteStorage.one(r.id,one.host,one.port,one.username);
                if(rr.id>0){
                    one.pid = r.id;//set new pid
                    one.id = rr.id;//connect local data
                    siteStorage.update(one);
                }else{
                    one.id = 0l;
                    one.pid = r.id;
                    siteStorage.insert(one);
                }
            }
        }
    }


}

ProjectRecord BackupRestore::findProject(long long id){
    for(auto one:d->projects){
        if(one.id==id){
            return one;
        }
    }
    return {};
}

SiteRecord BackupRestore::findSite(long long id){
    for(auto one:d->sites){
        if(one.id==id){
            return one;
        }
    }
    return {};
}

QString BackupRestore::encode(const QString& str){
    return DES::encode(str,key,vt);
}

QString BackupRestore::decode(const QString& str){
    return DES::decode(str,key,vt);
}

QString BackupRestore::defaultFilename(){
    return APP_NAME+"-export-"+QDateTime::currentDateTime().toString("yyyyMMdd")+".json";
}

}
