#ifndef UTILS_H
#define UTILS_H
#include <QString>
#include "global.h"
namespace ady {
    class ANYENGINE_EXPORT Utils
    {
    public:
        static QString readableFilesize(quint64 filesize,int decimal=2);
        static QString matchRemoteFilePath(const QString& local,const QString& projectDir,const QString& siteDir);
        static QString keepMatchFilePath(const QString& src,const QString& projectDir,const QString& siteDir);
        static bool allowExtension(const QString& ext,QStringList exts);
        static QString toUnixPath(QString path);
        static QString toLocalPath(QString path);
        static const char* toUTF8(const QString& str);
        static void deleteFile(const QString& path);
        static bool isFilename(const QString& name);
        static QStringList split(const QString& str,const QString& sep,int limit=0);
        static QStringList split(const QString& str,QRegExp sep,int limit=0);
        static bool copy(const QString& oDir,const QString& nDir);
        static QString stringToExpression(const QString &original);


    };
}

#endif // UTILS_H
