#include "DataImport.h"
#include "../export/io_common.h"
#include "core/des.h"
#include <QFileDialog>
#include <QDateTime>
#include <QDebug>



namespace ady {
DataImport::DataImport(){
    pReader = nullptr;
}

DataImport::~DataImport(){
    if(pReader!=nullptr){
        delete pReader;
        pReader = nullptr;
    }
}

int DataImport::doImport(QWidget* parent)
{
    QString filename = QFileDialog::getOpenFileName(parent,QObject::tr("Open File"),"",QObject::tr("Ady File (*.ady)"));
    if(!filename.isEmpty()){
        QFile inputFile(filename);
        if(inputFile.exists()){
            if(inputFile.open(QIODevice::ReadOnly)){
                pReader = new QXmlStreamReader(&inputFile);

                while(pReader->readNextStartElement()){
                    if(pReader->name()!=ADY_TAG){
                        pReader->raiseError(QObject::tr("Invalid AnyDeploy Export File!"));
                        return 1;//invalid file;
                    }else if(pReader->isStartElement()){
                        QXmlStreamAttributes attrs = pReader->attributes();
                        //qDebug()<<attrs.toList().length();
                        if(attrs.hasAttribute(ADY_ATTR_VERSION)){
                            QStringRef v = attrs.value(ADY_ATTR_VERSION);
                            bool ok = false;
                            this->version = v.toInt(&ok);
                            if(ok){
                                bool ret = this->readBaseInfo();
                                //qDebug()<<"ret:"<<ret;
                                if(ret){
                                    //read data
                                    if(this->readData()){
                                        //qDebug()<<"data count:"<<this->data.length();
                                        return 0;
                                        //return importData();
                                    }

                                }
                            }
                        }
                    }
                }
            }
        }
        return 1;

    }else{
        return -1;
    }
}

bool DataImport::readBaseInfo()
{

    if(pReader->readNextStartElement()){
        if(pReader->name()==INFO_TAG && pReader->isStartElement()){
            while(pReader->readNext()){
                if(pReader->name()==NAME_TAG && pReader->isStartElement()){
                    this->name = pReader->readElementText();
                }else if(pReader->name()==CREATOR_TAG && pReader->isStartElement()){
                    this->creator = pReader->readElementText();
                }
                if(pReader->name()==INFO_TAG && pReader->isEndElement()){
                    return true;
                }
            }
        }

        return false;
    }else{
        qDebug()<<"no next";
        return false;
    }
}

bool DataImport::readData()
{
    if(this->pReader->readNextStartElement() && pReader->name()==DATA_TAG && pReader->isStartElement()){
        return this->readProjects();
    }
    return false;
}

bool DataImport::readProjects()
{
    while(pReader->readNext()){
        if(pReader->name()==PROJECTS_TAG){
            if(pReader->isStartElement()){
                return this->readProject();
            }
        }
    }
    return false;
}

bool DataImport::readProject()
{
    bool projectOpen = false;
    DataImportProject* p = nullptr;
    while(pReader->readNext()){
        if(pReader->name()==PROJECT_TAG){
            if(pReader->isStartElement()){
                projectOpen = true;
                p = new DataImportProject();
            }else if(pReader->isEndElement()){
                if(p!=nullptr){
                    DataImportProject one = *p;
                    this->data.push_back(one);
                    delete p;
                }
                p = nullptr;
                projectOpen = false;
            }
        }else if(pReader->name()==PROJECTS_TAG &&pReader->isEndElement()){
            return true;
        }
        if(projectOpen){
            if(pReader->isStartElement()){
                if(pReader->name()==PROJECT_NAME_TAG){
                    p->project.name = pReader->readElementText();
                }else if(pReader->name()==PROJECT_PATH_TAG){
                    p->project.path = pReader->readElementText();
                }else if(pReader->name()==GROUPS_TAG){
                    this->readGroups(p->groups);
                }
            }
        }
    }
    return false;
}

bool DataImport::readGroups(QList<DataImportGroup>& lists)
{
    bool groupOpen = false;
    DataImportGroup* g = nullptr;
    while(pReader->readNext()){
        if(pReader->name()==GROUP_TAG){
            if(pReader->isStartElement()){
                groupOpen = true;
                QXmlStreamAttributes attrs = pReader->attributes();
                g = new DataImportGroup();
                if(attrs.hasAttribute(GROUP_ATTR_SYSTEM)){
                    QStringRef v = attrs.value(GROUP_ATTR_SYSTEM);
                    bool ok = false;
                    int system = v.toInt(&ok);
                    if(ok){
                        g->group.id = system;
                    }else{
                        g->group.id = 0;
                    }
                }else{
                    g->group.id = 0;
                }
            }else if(pReader->isEndElement()){
                if(g!=nullptr){
                    DataImportGroup one = *g;
                    lists.push_back(one);
                    delete  g;
                }
                g = nullptr;
                groupOpen = false;
            }
        }else if(pReader->name()==GROUPS_TAG && pReader->isEndElement()){
            return true;
        }

        if(groupOpen){
            if(pReader->isStartElement()){
                if(pReader->name()==GROUP_NAME_TAG){
                    g->group.name = pReader->readElementText();
                }else if(pReader->name()==SITES_TAG){
                    this->readSites(g->sites);
                }
            }
        }


    }
    return false;
}

bool DataImport::readSites(QList<SiteRecord>& lists)
{
    bool siteOpen = false;
    SiteRecord* s = nullptr;
    while(pReader->readNext()){
        if(pReader->name()==SITE_TAG){
            if(pReader->isStartElement()){
                siteOpen = true;

                s = new SiteRecord();


            }else if(pReader->isEndElement()){
                if(s!=nullptr){
                    SiteRecord one = *s;
                    lists.push_back(one);
                    delete  s;
                }
                s = nullptr;
                siteOpen = false;
            }
        }else if(pReader->name()==SITES_TAG && pReader->isEndElement()){
            return true;
        }

        if(siteOpen){
            if(pReader->isStartElement()){
                if(pReader->name()==SITE_NAME_TAG){
                    s->name = pReader->readElementText();
                }else if(pReader->name()==SITE_HOST_TAG){
                    s->host = pReader->readElementText();
                }else if(pReader->name()==SITE_PORT_TAG){
                    QString str = pReader->readElementText();
                    s->port = str.toInt();
                }else if(pReader->name()==SITE_USERNAME_TAG){
                    s->username = pReader->readElementText();
                }else if(pReader->name()==SITE_PASSWORD_TAG){
                    QString password = pReader->readElementText();
                    qDebug()<<"p1:"<<password;
                    s->password = DES::decode(password);
                    qDebug()<<"p2:"<<s->password;
                }else if(pReader->name()==SITE_PATH_TAG){
                    s->path = pReader->readElementText();
                }else if(pReader->name()==SITE_TYPE_TAG){
                    s->type = pReader->readElementText();
                }else if(pReader->name()==SITE_STATUS_TAG){
                    s->status = pReader->readElementText().toInt();
                }else if(pReader->name()==SITE_LISTORDER_TAG){
                    s->listorder = pReader->readElementText().toInt();
                }else if(pReader->name()==SITE_SETTINGS_TAG){
                    s->settings = pReader->readElementText();
                }
            }
        }
    }

    return false;
}



int DataImport::importData(){
    //insert data
    ProjectStorage projectStorage;
    GroupStorage groupStorage;
    SiteStorage siteStorage;


    Q_FOREACH(DataImportProject p,this->data){
        p.project.datetime = QDateTime::currentDateTime().toSecsSinceEpoch();
        long long pid = projectStorage.insert(p.project);
        //qDebug()<<"pid:"<<pid;
        if(pid>0){
            Q_FOREACH(DataImportGroup g,p.groups){
                long long gid = g.group.id;
                if(g.group.id==0){
                    g.group.pid = pid;
                    gid = groupStorage.insert(g.group);
                }
                //qDebug()<<"gid:"<<gid;
                if(gid>0){
                    Q_FOREACH(SiteRecord r,g.sites){
                        r.pid = pid;
                        r.groupid = gid;
                        r.datetime = p.project.datetime;
                        long long id = siteStorage.insert(r);
                        qDebug()<<"id:"<<id;
                    }
                }
            }
        }
    }
    return 0;
}




}
