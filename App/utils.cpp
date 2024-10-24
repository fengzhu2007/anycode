#include "utils.h"
#include <math.h>
#include <QStringList>
#include <QFileInfo>
#include <QDir>
#include <QRegularExpression>
#include <QTextCodec>

#include <QDebug>
namespace ady {

    QString Utils::readableFilesize(quint64 filesize)
    {
        QStringList units;
        units << "B" << "KB" << "MB" << "GB" << "TB" << "PB";
        double mod  = 1024.0;
        double size = filesize;
        //qDebug() << size;
        int i = 0;
        long rest = 0;
        while (size >= mod && i < units.count()-1 )
        {
            rest= (long)size % (long)mod ;
            size /= mod;
            i++;
        }
        QString szResult = QString::number(floor(size));
        if( rest > 0)
        {
           szResult += QString(".") + QString::number(rest).left(2);
        }
        szResult += units[i];
        return  szResult;
    }

    QString Utils::toUnixPath(QString path)
    {
#ifdef Q_OS_WIN32
        return path.replace('\\','/',path);
#else
        return path;
#endif
    }

    QString Utils::toLocalPath(QString path)
    {
#ifdef Q_OS_WIN32
        return path.replace('/','\\',path);
#else
        return path;
#endif
    }

    const char* Utils::toUTF8(QString str)
    {
#ifdef Q_OS_WIN32
        return str.toUtf8().constData();
        QTextCodec* codec = QTextCodec::codecForLocale();
        QByteArray by = codec->fromUnicode(str);
        return by.constData();
#else
        return str.toStdString().c_str();
#endif
    }

    void Utils::deleteFile(QString path)
    {
        QFileInfo fi(path);
        if(fi.exists()){
            if(fi.isDir()){
                QDir d(path);
                d.removeRecursively();
                /*QFileInfoList lists = d.entryInfoList(QDir::AllEntries|QDir::NoDotAndDotDot|QDir::Hidden);
                int length = lists.length();
                for(int i=0;i<length;i++){
                    QFileInfo fi = lists.at(i);
                    Utils::deleteFile(fi.filePath());
                }
                d.remove();*/
            }else{
                QFile::remove(path);
            }

        }
    }

    bool Utils::isFilename(QString name)
    {
        QRegularExpression re("^[^/\\\\:\\*\\?\\<\\>\\|\"]{1,255}$");
        return re.match(name).hasMatch();
    }


    QStringList Utils::split(const QString& str,const QString& sep,int limit)
    {
        if(limit<=0){
            return str.split(sep);
        }else{
            int length = sep.length();
            QStringList lists;
            QString stri = str;
            while(true){
                int pos = str.indexOf(sep);
                if(lists.length() == limit - 1){
                    pos = -1;
                }
                if(pos==-1){
                    lists.push_back(stri);
                    break;
                }else{
                    lists.push_back(stri.left(pos));
                    stri = stri.mid(pos + length);
                }
            }
            //qDebug()<<"lists:"<<lists;
            return lists;

        }
    }

    QStringList Utils::split(const QString& str,QRegExp sep,int limit)
    {
        if(limit<=0){
            return str.split(sep);
        }else{
            //int length = sep.length();
            QStringList lists;
            QString stri = str;
            int pos = 0;
            //int start = 0;
            while ((pos = sep.indexIn(stri)) != -1) {
                //qDebug()<<"str:"<<stri;
                //qDebug()<<"pos:"<<pos;
                 lists.push_back(stri.left(pos));
                 pos += sep.matchedLength();
                 stri = stri.mid(pos);
                 if(lists.length()==limit - 1){
                     break;
                 }
            }
            lists.push_back(stri);
            //qDebug()<<"lists:"<<lists;
            return lists;

        }
    }
}
