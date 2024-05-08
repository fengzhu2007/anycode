#ifndef COSSETTING_H
#define COSSETTING_H
#include "storage/SiteSetting.h"

namespace ady {



    class COSSetting
    {
    public:
        COSSetting(SiteSetting& setting);
        ~COSSetting();
        QList<QPair<QString,QString>> dirSync();
        QList<QPair<QString,QString>> dirMapping();
        QStringList filterExtensions();
        QStringList filterDirs();


    private:
        QMap<QString,void*> m_data;



    };
}
#endif // COSSETTING_H
