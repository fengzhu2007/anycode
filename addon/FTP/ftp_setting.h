#ifndef FTPSETTING_H
#define FTPSETTING_H
#include "storage/site_setting.h"

namespace ady {



    class FTPSetting
    {
    public:
        FTPSetting(const SiteSetting& setting);
        ~FTPSetting();
        QList<QPair<QString,QString>> dirSync();
        QList<QPair<QString,QString>> dirMapping();
        long long interval();



    private:
        QMap<QString,void*> m_data;



    };
}
#endif // FTPSETTING_H
