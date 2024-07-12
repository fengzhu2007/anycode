#ifndef UTILS_H
#define UTILS_H
#include <QString>
#include "global.h"
namespace ady {
    class ANYENGINE_EXPORT Utils
    {
    public:
        static QString readableFilesize(quint64 filesize);
        static QString matchRemoteFilePath(QString local,QString projectDir,QString siteDir);
        static QString keepMatchFilePath(QString src,QString projectDir,QString siteDir);
        static bool allowExtension(QString ext,QStringList exts);
        static QString toUnixPath(QString path);
        static QString toLocalPath(QString path);
        static const char* toUTF8(QString str);
        static void deleteFile(QString path);
        static bool isFilename(QString name);
        static QStringList split(const QString& str,const QString& sep,int limit=0);
        static QStringList split(const QString& str,QRegExp sep,int limit=0);
        static bool copy(const QString& oDir,const QString& nDir);
    };
}

#endif // UTILS_H
