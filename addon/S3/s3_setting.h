#ifndef S3SETTING_H
#define S3SETTING_H
#include "storage/site_setting.h"

namespace ady {



    class S3Setting
    {
    public:
        S3Setting(const SiteSetting& setting);
        ~S3Setting();
        QList<QPair<QString,QString>> dirSync();
        QList<QPair<QString,QString>> dirMapping();
        QStringList filterExtensions();
        QStringList filterDirs();


    private:
        QMap<QString,void*> m_data;



    };
}
#endif // S3SETTING_H
