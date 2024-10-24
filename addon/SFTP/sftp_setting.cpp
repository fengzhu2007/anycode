#include "sftp_setting.h"
#include "sftp_setting_key.h"
#include <QJsonArray>
#include <QJsonObject>
namespace ady {

SFTPSetting::SFTPSetting(SiteSetting & setting){
    {
        QJsonValue value = setting.get(SFTP_LOCAL_DIR_SYNC);
        if(!value.isNull() && value.toArray().size()>0){
            QList<QPair<QString,QString>>* ptr = new QList<QPair<QString,QString>>();
            QJsonArray arr = value.toArray();
            for(auto one:arr){
                if(!one.isNull()){
                    QJsonObject object = one.toObject();
                    if(object.contains(SRC) && object.contains(DST)){
                        ptr->push_back(qMakePair(object.value(SRC).toString(),object.value(DST).toString()));
                    }
                }
            }
            m_data.insert(SFTP_LOCAL_DIR_SYNC,ptr);
        }
    }


    {
        QJsonValue value = setting.get(SFTP_REMOTE_DIR_MAPPING);
        if(!value.isNull() && value.toArray().size()>0){
            QList<QPair<QString,QString>>* ptr = new QList<QPair<QString,QString>>();
            QJsonArray arr = value.toArray();
            for(auto one:arr){
                if(!one.isNull()){
                    QJsonObject object = one.toObject();
                    if(object.contains(SRC) && object.contains(DST)){
                        ptr->push_back(qMakePair(object.value(SRC).toString(),object.value(DST).toString()));
                    }
                }
            }
            m_data.insert(SFTP_REMOTE_DIR_MAPPING,ptr);
        }
    }

    {
        QJsonValue value = setting.get(SFTP_UPLOAD_COMMANDS);
        if(!value.isNull()){
            QString *ptr = new QString();
            *ptr = value.toString();
            m_data.insert(SFTP_UPLOAD_COMMANDS,ptr);
        }

    }



}

SFTPSetting::~SFTPSetting()
{
    QMap<QString,void*>::iterator iter = m_data.begin();
    while(iter!=m_data.end()){
        QString key = iter.key();
        if(key==SFTP_LOCAL_DIR_SYNC || key==SFTP_REMOTE_DIR_MAPPING){
            delete (QList<QPair<QString,QString>>*)iter.value();
        }else if(key==SFTP_UPLOAD_COMMANDS){
            delete (QString*)iter.value();
        }
        iter++;
    }
}

QList<QPair<QString,QString>> SFTPSetting::dirSync()
{
    if(m_data.contains(SFTP_LOCAL_DIR_SYNC)){
        QList<QPair<QString,QString>>* ptr = (QList<QPair<QString,QString>>*)m_data[SFTP_LOCAL_DIR_SYNC];
        return *ptr;
    }else{
        return QList<QPair<QString,QString>>();
    }
}

QList<QPair<QString,QString>> SFTPSetting::dirMapping()
{
    if(m_data.contains(SFTP_REMOTE_DIR_MAPPING)){
        QList<QPair<QString,QString>>* ptr = (QList<QPair<QString,QString>>*)m_data[SFTP_REMOTE_DIR_MAPPING];
        return *ptr;
    }else{
        return QList<QPair<QString,QString>>();
    }
}

QStringList SFTPSetting::uploadCommands()
{
    if(m_data.contains(SFTP_UPLOAD_COMMANDS)){
        QString* commands = (QString*)m_data[SFTP_UPLOAD_COMMANDS];
        if(!commands->isEmpty()){
            return commands->split("\n");
        }
    }
    return QStringList();
}

}
