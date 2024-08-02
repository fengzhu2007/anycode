#include "DataExport.h"
#include "io_common.h"
#include "common.h"
#include <QFileDialog>
#include <QWidget>
#include <QDateTime>
#include <QDebug>

#include "storage/project_storage.h"
#include "storage/group_storage.h"
#include "storage/site_storage.h"

#include "core/des.h"

/*
xml simle
<?xml >
<ADY>
    <INFO>
        <NAME>AnyDeploy Export Data</NAME>
        <VERSION></VERSION>
        <CREATOR></CREATOR>
        <DATE></DATE>
    </INFO>
    <DATA>
        <PROJECTS>
            <PROJECT>
                <NAME></NAME>
                <PATH></PATH>
                <GROUPS>
                    <GROUP system="0,1,2,3">
                        <NAME ></NAME>
                        <SITES>
                            <SITE>
                                <NAME></NAME>
                                <HOST></HOST>
                                <PORT></PORT>
                                <USERNAME><USERNAME>
                                <PASSWORD></PASSWORD>
                                <PATH></PATH>
                                <TYPE></TYPE>
                                <STATUS></STATUS>
                                <LISTORDER></LISTORDER>
                                <SETTINGS></SETTINGS>
                            </SITE>
                        </SITES>
                    </GROUP>
                </GROUPS>
            </PROJECT>

        </PROJECTS>

    </DATA>

</ADY>


*/


const QString ADY_EXPORT_NAME = "AnyDeploy Export Data";

namespace ady {
    DataExport::DataExport(){
        pWriter = nullptr;
    }

    DataExport:: ~DataExport()
    {
        if(pWriter!=nullptr){
            delete pWriter;
            pWriter = nullptr;
        }
    }

    int DataExport::doExport(QWidget *parent)
    {
        QString filename = QFileDialog::getSaveFileName(parent,QObject::tr("Save File"),"",QObject::tr("Ady File (*.ady)"));
        if(!filename.isEmpty()){
            QFile outputFile(filename);
            if(outputFile.open(QIODevice::WriteOnly)){
                pWriter = new QXmlStreamWriter(&outputFile);
                pWriter->setAutoFormatting(true);
                pWriter->writeStartDocument();
                pWriter->writeStartElement(ADY_TAG);
                pWriter->writeAttribute(ADY_ATTR_VERSION,QString("%1").arg(version));
                this->writeBaseInfo();
                this->writeData();
                pWriter->writeEndElement();
                pWriter->writeEndDocument();
                return 0;
            }
        }else{
            return -1;//cancel
        }
        return 1;//error failed
    }

    void DataExport::writeBaseInfo()
    {
        pWriter->writeStartElement(INFO_TAG);
        pWriter->writeTextElement(NAME_TAG,ADY_EXPORT_NAME);
        pWriter->writeTextElement(CREATOR_TAG,APP_NAME +" "+ APP_VERSION);
        pWriter->writeTextElement(DATE_TAG,QDateTime::currentDateTime().toString(Qt::ISODate));
        pWriter->writeEndElement();
    }

    void DataExport::writeData()
    {

        pWriter->writeStartElement(DATA_TAG);
        ProjectStorage projectStorage;
        QList<ProjectRecord>projects = projectStorage.all();

        GroupStorage groupStorage;
        QList<GroupRecord>groups = groupStorage.all();

        SiteStorage siteStorage;
        QList<SiteRecord> sites = siteStorage.all();


        pWriter->writeStartElement(PROJECTS_TAG);

        Q_FOREACH(ProjectRecord p ,projects){
            pWriter->writeStartElement(PROJECT_TAG);
            pWriter->writeTextElement(PROJECT_NAME_TAG,p.name);
            pWriter->writeTextElement(PROJECT_PATH_TAG,p.path);

            pWriter->writeStartElement(GROUPS_TAG);//groups start

            Q_FOREACH(GroupRecord g,groups){
                if(g.pid==p.id){
                    pWriter->writeStartElement(GROUP_TAG);
                    pWriter->writeAttribute(GROUP_ATTR_SYSTEM,"0");
                }else if(g.pid==0){
                    pWriter->writeStartElement(GROUP_TAG);
                    pWriter->writeAttribute(GROUP_ATTR_SYSTEM,QString("%1").arg(g.id));
                }else{
                    continue;
                }
                pWriter->writeTextElement(GROUP_NAME_TAG,g.name);

                pWriter->writeStartElement(SITES_TAG);//sites start

                Q_FOREACH(SiteRecord r,sites){
                    if(r.pid==p.id && r.groupid==g.id){
                        pWriter->writeStartElement(SITE_TAG);//site start
                        {
                            this->writeDataNode(SITE_NAME_TAG,r.name);//name
                            pWriter->writeTextElement(SITE_HOST_TAG,r.host);
                            pWriter->writeTextElement(SITE_PORT_TAG,QString("%1").arg(r.port));
                            this->writeDataNode(SITE_USERNAME_TAG,r.username);//username
                            pWriter->writeTextElement(SITE_PASSWORD_TAG,DES::encode(r.password));
                            pWriter->writeTextElement(SITE_PATH_TAG,r.path);
                            pWriter->writeTextElement(SITE_TYPE_TAG,r.type);
                            pWriter->writeTextElement(SITE_STATUS_TAG,QString("%1").arg(r.status));
                            pWriter->writeTextElement(SITE_LISTORDER_TAG,QString("%1").arg(r.listorder));
                            this->writeDataNode(SITE_SETTINGS_TAG,r.settings);//settings
                        }
                        pWriter->writeEndElement();//site end
                    }
                }

                pWriter->writeEndElement();//sites end
                pWriter->writeEndElement();//group end
            }

            pWriter->writeEndElement();//groups end


            pWriter->writeEndElement();//project end
        }


        pWriter->writeEndElement();//projets end

        pWriter->writeEndElement();//data end

    }

    void DataExport::writeDataNode(const QString& name,const QString& text)
    {
        pWriter->writeStartElement(name);//start
        pWriter->writeCDATA(text);
        pWriter->writeEndElement();//end
    }
}
