#ifndef FTP_PARSE_H
#define FTP_PARSE_H
#include <QString>
#include <QMap>
#include "interface/file_item.h"
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
#endif // FTP_PARSE_H
