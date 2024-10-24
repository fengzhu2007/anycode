#ifndef OSS_SETTING_H
#define OSS_SETTING_H
#include "storage/SiteSetting.h"

namespace ady {



    class OSSSetting
    {
    public:
        OSSSetting(const SiteSetting& setting);
        ~OSSSetting();
        QList<QPair<QString,QString>> dirSync();
        QList<QPair<QString,QString>> dirMapping();
        QStringList filterExtensions();
        QStringList filterDirs();



    private:
        QMap<QString,void*> m_data;



    };
}
#endif // OSS_SETTING_H
