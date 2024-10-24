#ifndef SFTPPARSE_H
#define SFTPPARSE_H
#include <QString>
#include <QMap>
#include "local/file_item.h"
namespace ady {
    class SFTPParse
    {
    public:

        SFTPParse();
        bool lineParse(QString& line,FileItem& entry);


    private:

        bool parseAsUnix(QString &line, FileItem& entry);
        //bool parseAsMlsd(QString &line, FileItem& entry);
        bool parseAsDos(QString &line, FileItem &entry);

        bool parseShortDate(QString &date,QString &time,FileItem &entry);



    public:
        static QMap<QString,int> monthNameMap;
    };
}
#endif // SFTPPARSE_H
