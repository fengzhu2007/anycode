#ifndef SFTPSETTING_H
#define SFTPSETTING_H
#include "storage/site_setting.h"

namespace ady {



    class SFTPSetting
    {
    public:
        SFTPSetting(SiteSetting& setting);
        ~SFTPSetting();
        QList<QPair<QString,QString>> dirSync();
        QList<QPair<QString,QString>> dirMapping();
        QStringList uploadCommands();



    private:
        QMap<QString,void*> m_data;



    };
}
#endif // SFTPSETTING_H
