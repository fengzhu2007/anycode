#ifndef FTPSETTING_H
#define FTPSETTING_H
#include "storage/SiteSetting.h"

namespace ady {



    class FTPSetting
    {
    public:
        FTPSetting(SiteSetting& setting);
        ~FTPSetting();
        QList<QPair<QString,QString>> dirSync();
        QList<QPair<QString,QString>> dirMapping();



    private:
        QMap<QString,void*> m_data;



    };
}
#endif // FTPSETTING_H
