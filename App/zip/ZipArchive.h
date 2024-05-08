#ifndef ZIPARCHIVE_H
#define ZIPARCHIVE_H
#include <QObject>
#include "global.h"

#include "minizip/zip.h"


namespace ady {

class ANYENGINE_EXPORT ZipArchive : public QObject
{

public:
    enum Mode{
        Normal=0,
        Append,
        Overwrite,
    };
    ZipArchive(QObject* parent=nullptr);
    bool open(QString pathname,Mode mode=Overwrite);
    bool open(QString pathname,QString password,Mode mode=Overwrite);
    bool addFile(QString pathname,QString filename);
    bool addContent(QByteArray ba,QString filename);
    bool addFolder(QString dirname);
    bool close();
    inline int errorCode(){return m_errorCode;}

private:
    zipFile m_zf;
    int m_errorCode;
    QString m_password;

};


}





#endif // ZIPARCHIVE_H
