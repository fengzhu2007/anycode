#include "ftp_parse.h"
#include <QDate>
#include <QDebug>
#include "common/utils.h"
namespace ady {
QMap<QString,int> FTPParse::monthNameMap;




FTPParse::FTPParse(bool mlsd)
    :mlsd(mlsd)
{
    if(monthNameMap.isEmpty()){

        monthNameMap["jan"] = 1;
        monthNameMap["feb"] = 2;
        monthNameMap["mar"] = 3;
        monthNameMap["apr"] = 4;
        monthNameMap["may"] = 5;
        monthNameMap["jun"] = 6;
        monthNameMap["june"] = 6;
        monthNameMap["jul"] = 7;
        monthNameMap["july"] = 7;
        monthNameMap["aug"] = 8;
        monthNameMap["sep"] = 9;
        monthNameMap["sept"] = 9;
        monthNameMap["oct"] = 10;
        monthNameMap["nov"] = 11;
        monthNameMap["dec"] = 12;

        monthNameMap["1"] = 1;
        monthNameMap["01"] = 1;
        monthNameMap["2"] = 2;
        monthNameMap["02"] = 2;
        monthNameMap["3"] = 3;
        monthNameMap["03"] = 3;
        monthNameMap["4"] = 4;
        monthNameMap["04"] = 4;
        monthNameMap["5"] = 5;
        monthNameMap["05"] = 5;
        monthNameMap["6"] = 6;
        monthNameMap["06"] = 6;
        monthNameMap["7"] = 7;
        monthNameMap["07"] = 7;
        monthNameMap["8"] = 8;
        monthNameMap["08"] = 8;
        monthNameMap["9"] = 9;
        monthNameMap["09"] = 9;
        monthNameMap["10"] = 10;
        monthNameMap["11"] = 11;
        monthNameMap["12"] = 12;
    }
}


bool FTPParse::lineParse(QString& line,FileItem& entry)
{
    if(this->mlsd){
        return parseAsMlsd(line,entry);
    }else{
        if(!parseAsUnix(line,entry)){
            if(!parseAsDos(line,entry)){
                return false;
            }
        }
        return true;
    }
}






bool FTPParse::parseAsUnix(QString &line, FileItem& entry)
{
    //QStringList tokens = line.split(QRegExp("\\s+"));
    QStringList tokens = Utils::split(line,QRegExp("\\s+"),8);
    int size = tokens.size();
    QString permission;
    if(size>0){
     permission = tokens.at(0);
    }
    if(permission.isEmpty()){
      return false;
    }
    QChar chr = permission[0];
    if (chr != 'b' && chr != 'c' &&chr != 'd' &&chr != 'l' &&chr != 'p' &&chr != 's' &&chr != '-'){
        return false;
    }
    if(permission[0]=='d'){
        entry.type = FileItem::Dir;
    }else if(permission[0]=='-'){
        entry.type = FileItem::File;
    }
    entry.permission = permission;


    if(size>4){
        entry.size = tokens.at(4).toInt();
    }else{
        entry.size = 0;
    }
    QString month;
    int iMonth = 1;
    if(size>5){
        month = tokens.at(5).toLower();
        iMonth = monthNameMap[month];
    }
    int iDay = 1;
    if(size>6){
        iDay = tokens.at(6).toInt();
    }
    QString year_or_time;
    if(size>7){

        QString year_or_time_and_name = tokens[7];
        QStringList arr = Utils::split(year_or_time_and_name," ",2);
         year_or_time = arr[0];
         if(arr.length()>0){
             entry.name = arr[1];
         }
    }

    if(year_or_time.indexOf(":")==-1){
        entry.modifyTime = QString("%1/%2/%3").arg(year_or_time).arg(iMonth,2,10,QLatin1Char('0')).arg(iDay,2,10,QLatin1Char('0'));
    }else{
        QStringList time = year_or_time.split(":");
        int hour = time[0].toInt();
        int min = time[1].toInt();
        QDate today = QDate::currentDate();
        int year = today.month() < iMonth?today.year()-1:today.year();
        entry.modifyTime = QString("%1/%2/%3 %4:%5").arg(year).arg(iMonth,2,10,QLatin1Char('0')).arg(iDay,2,10,QLatin1Char('0')).arg(hour,2,10,QLatin1Char('0')).arg(min,2,10,QLatin1Char('0'));

    }
    /*if(size>8){
        entry.name = tokens.at(8);
    }*/
    return true;
}

bool FTPParse::parseAsMlsd(QString &line, FileItem& entry)
{
    //tokens_and_name = line.split("\\s",2);
    QStringList tokens_and_name = Utils::split(line," ",2);
    QStringList tokens = tokens_and_name[0].split(";");
    int size = tokens.size();
    if(size>=2){
        entry.name = tokens_and_name[1];
    }else{
        return false;
    }
    for(int i=0;i<size - 1;i++){
        QString str = tokens[i];
        int pos = str.indexOf("=");
        if(pos!=-1){
            QString key = str.left(pos).toLower();
            QString value = str.mid(pos + 1);
            //qDebug()<<key<<":"<<value;
            if(key=="type"){
                if(value=="dir" || value =="cdir" || value=="pdir"){
                    entry.type = FileItem::Dir;
                }else{
                    entry.type = FileItem::File;
                }
            }else if(key=="sizd" || key=="size"){
                entry.size = value.toInt();
            }else if(key=="modify"){
                entry.modifyTime = QString("%1/%2/%3 %4:%5").arg(value.left(4)).arg(value.mid(4,2)).arg(value.mid(6,2)).arg(value.mid(8,2)).arg(value.mid(10,2));
            }else if(key=="unix.mode"){
                entry.permission = value;
            }

        }
    }
    return true;

}

bool FTPParse::parseAsDos(QString &line, FileItem &entry)
{
    QStringList tokens = Utils::split(line,QRegExp("\\s+"),3);
   // qDebug()<<"line:"<<line;
    //qDebug()<<"tokens:"<<tokens;
    //QStringList tokens = line.split(QRegExp("\\s+"));
    int size = tokens.size();
    if(size<3){
        return false;
    }
    QString date = tokens.at(0);
    QString time = tokens.at(1);
    //QString dir = tokens.at(2);
    QString size_and_name = tokens.at(2);
    // qDebug()<<"size and name:"<<size_and_name;
    if(size_and_name.startsWith("<DIR>")){
        QStringList arr = Utils::split(size_and_name,QRegExp("\\s{10}"),2);
        entry.type = FileItem::Dir;
        entry.size = 0;
        entry.name = arr.at(1);
    }else{
        QStringList arr = Utils::split(size_and_name," ",2);
        QString dir = arr.at(0);
        entry.name = arr.at(1);
        entry.type = FileItem::File;
        entry.size = dir.toInt();
    }
    if(!this->parseShortDate(date,time,entry)){
        return false;
    }

    return true;
}


bool FTPParse::parseShortDate(QString &date,QString &time,FileItem &entry)
{
    if(date.isEmpty()){
            return false;
    }
    QStringList arr;
    int pos = -1;
    if((pos = date.indexOf("-"))>-1){
        arr = date.split("-");
    }else if((pos = date.indexOf("."))>-1){
        arr = date.split(".");
    }else if((pos = date.indexOf("/"))>-1){
        arr = date.split(("/"));
    }else{
        return false;
    }
    if(arr.size()!=3){
        return false;
    }
    int tpos = -1;
    if((tpos = time.indexOf(":"))==-1){
        return false;
    }
    QString yyyy;
    QString mm;
    QString dd;



        if(pos==4){
            //yyyy-mm-dd
            yyyy = arr[0];
            mm = arr[1];
            dd = arr[2];
        }else if(pos<=2){
            if(date[pos]=='.'){
                //dd.mm.yyyy
                yyyy = arr[2];
                mm = arr[1];
                dd = arr[0];
            }else if(date[pos]=='-'||date[pos]=='/'){
                //mm-dd-yy
                /*if(arr[2].size()==4){
                    //dd-mm-yyyy
                    yyyy = arr[2];
                    mm = arr[1];
                    dd = arr[0];
                }else */if(arr[0].size()==4){
                    //yyyy-mm-dd
                    yyyy = arr[0];
                    mm = arr[1];
                    dd = arr[2];
                }else{
                     //mm-dd-yy
                    yyyy = arr[2];
                    mm = arr[0];
                    dd = arr[1];
                }
            }
        }


        int year = yyyy.toInt();
        if(yyyy.size()==2){
            if(year<50){
                year += 2000;
            }else{
                year += 1900;
            }
        }
        int month = monthNameMap[mm];

        QString hh = time.left(tpos);
        QString ii = time.mid(tpos+1,2);
        QString ap = time.mid(tpos);
        int hour = hh.toInt();
        if(ap=="PM"||ap=="pm"){
            hour += 12;
        }
        entry.modifyTime = QString("%1/%2/%3 %4:%5").arg(year).arg(month,2,10,QLatin1Char('0')).arg(dd).arg(hour,2,10,QLatin1Char('0')).arg(ii);
        return true;
}

}
