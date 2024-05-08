#ifndef {TPL}SETTING_H
#define {TPL}SETTING_H
#include "storage/SiteSetting.h"

namespace ady {



    class {TPL}Setting
    {
    public:
        {TPL}Setting(SiteSetting& setting);
        ~{TPL}Setting();
        QList<QPair<QString,QString>> dirSync();
        QList<QPair<QString,QString>> dirMapping();



    private:
        QMap<QString,void*> m_data;



    };
}
#endif // {TPL}SETTING_H
