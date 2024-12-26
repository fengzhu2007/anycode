#ifndef {TPL}PARSE_H
#define {TPL}PARSE_H
#include <QString>
#include <QMap>
#include "local/file_item.h"
namespace ady {
    class {TPL}Parse
    {
    public:

        {TPL}Parse(bool mlsd=false);
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
#endif // {TPL}PARSE_H
