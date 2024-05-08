#ifndef CVS_COMMIT_H
#define CVS_COMMIT_H
#include "global.h"
#include <QString>
#include <QDateTime>
namespace ady {
namespace cvs {
    class ANYENGINE_EXPORT Commit
    {
    public:
        enum Flag {
            Dev=1,
            Prod=2,
            Other=4
        };
        Commit();
        Commit(QString oid,QString author,QDateTime time,QString content);
        inline void setAuthor(QString author){m_author = author;}
        inline void setEmail(QString email){m_email = email;}
        inline void setTime(QDateTime time){m_time = time;}
        inline void setContent(QString content){m_content = content;}
        inline void setOid(QString oid){m_oid = oid;}
        inline void setFlag(unsigned int flags){m_flags |= flags;}
        inline void setFlagValue(unsigned int flags){m_flags = flags;}

        inline QString author() const {return m_author;}
        inline QString email() const {return m_email;}
        inline QDateTime time() const {return m_time;}
        inline QString content() const {return m_content;}
        inline QString oid() const {return m_oid;}
        inline bool flag(Flag f) const {return (m_flags&f)==f;}
        inline unsigned int flag() const{return m_flags;}

    private:
        QString m_author;
        QString m_email;
        QDateTime m_time;
        QString m_oid;
        QString m_content;
        unsigned int m_flags;
    };
}
}
#endif // CVS_COMMIT_H
