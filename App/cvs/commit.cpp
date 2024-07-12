#include "commit.h"
namespace ady {
namespace cvs {
    Commit::Commit()
    {
        m_flags = 0;
    }

    Commit::Commit(QString oid,QString author,QDateTime time,QString content)
    {
        m_oid = oid;
        m_author = author;
        m_time = time;
        m_content = content;
        m_flags = 0;
    }

}
}
