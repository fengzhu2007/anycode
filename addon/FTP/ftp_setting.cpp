#include "ftp_setting.h"
#include "ftp_setting_key.h"
#include <QJsonArray>
#include <QJsonObject>
namespace ady {

FTPSetting::FTPSetting(const SiteSetting & setting){
    {
        QJsonValue value = setting.get(FTP_LOCAL_DIR_SYNC);
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
            m_data.insert(FTP_LOCAL_DIR_SYNC,ptr);
        }
    }


    {
        QJsonValue value = setting.get(FTP_REMOTE_DIR_MAPPING);
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
            m_data.insert(FTP_REMOTE_DIR_MAPPING,ptr);
        }
    }



}

FTPSetting::~FTPSetting()
{
    QMap<QString,void*>::iterator iter = m_data.begin();
    while(iter!=m_data.end()){
        QString key = iter.key();
        if(key==FTP_LOCAL_DIR_SYNC || key==FTP_REMOTE_DIR_MAPPING){
            delete (QList<QPair<QString,QString>>*)iter.value();
        }
        iter++;
    }
}

QList<QPair<QString,QString>> FTPSetting::dirSync()
{
    if(m_data.contains(FTP_LOCAL_DIR_SYNC)){
        QList<QPair<QString,QString>>* ptr = (QList<QPair<QString,QString>>*)m_data[FTP_LOCAL_DIR_SYNC];
        return *ptr;
    }else{
        return QList<QPair<QString,QString>>();
    }
}

long long FTPSetting::interval(){
    return 5 * 60 * 1000;//5 min
}

QList<QPair<QString,QString>> FTPSetting::dirMapping()
{
    if(m_data.contains(FTP_REMOTE_DIR_MAPPING)){
        QList<QPair<QString,QString>>* ptr = (QList<QPair<QString,QString>>*)m_data[FTP_REMOTE_DIR_MAPPING];
        return *ptr;
    }else{
        return QList<QPair<QString,QString>>();
    }
}

}
