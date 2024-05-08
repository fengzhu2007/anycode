#ifndef CVS_DIFFFILE_H
#define CVS_DIFFFILE_H
#include "global.h"
#include <QString>
namespace ady {
namespace cvs {
    class ANYENGINE_EXPORT DiffFile
    {
    public:
        enum Status {
            Normal=0,
            Addition,
            Deletion,
            Change
        };

        DiffFile(QString path,Status status);
        inline QString path() const {return m_path;}
        inline Status status() const {return m_status;}

        inline void setFilesize(long long filesize){
            m_filesize = filesize;
        }

        inline void setFiletime(QString filetime){
            m_filetime = filetime;
        }

        inline long long filesize() const{
            return m_filesize;
        }

        inline QString filetime() const {return m_filetime;}

    private:
        QString m_path;
        Status m_status;
        long long m_filesize = -1 ;
        QString m_filetime;


    };
}
}

#endif // CVS_DIFFFILE_H
