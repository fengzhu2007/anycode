#ifndef CVS_BRANCH_H
#define CVS_BRANCH_H
#include "global.h"
#include <QString>
namespace ady {
namespace cvs {
    class ANYENGINE_EXPORT Branch
    {
    public:
        explicit Branch(QString name,bool head=false);
        inline QString name(){return m_name;}
        inline bool head(){return m_head;}

    private:
        QString m_name;
        bool m_head;
    };
}
}
#endif // CVS_BRANCH_H
