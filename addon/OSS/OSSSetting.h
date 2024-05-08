#ifndef OSSSETTING_H
#define OSSSETTING_H
#include "storage/SiteSetting.h"

namespace ady {



    class OSSSetting
    {
    public:
        OSSSetting(SiteSetting& setting);
        ~OSSSetting();
        QList<QPair<QString,QString>> dirSync();
        QList<QPair<QString,QString>> dirMapping();
        QStringList filterExtensions();
        QStringList filterDirs();



    private:
        QMap<QString,void*> m_data;



    };
}
#endif // OSSSETTING_H
