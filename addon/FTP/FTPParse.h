#ifndef FTPPARSE_H
#define FTPPARSE_H
#include <QString>
#include <QMap>
#include "local/FileItem.h"
namespace ady {
    class FTPParse
    {
    public:

        FTPParse(bool mlsd=false);
        bool lineParse(QString& line,FileItem& entry);


    private:

        bool parseAsUnix(QString &line, FileItem& entry);
        bool parseAsMlsd(QString &line, FileItem& entry);
        bool parseAsDos(QString &line, FileItem &entry);

        bool parseShortDate(QString &date,QString &time,FileItem &entry);

    private:
        bool mlsd = false;

    public:
        static QMap<QString,int> monthNameMap;
    };
}
#endif // FTPPARSE_H
