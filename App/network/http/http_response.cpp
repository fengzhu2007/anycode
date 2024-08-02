#include "http_response.h"
#include <QDebug>
namespace ady{
    HttpResponse::HttpResponse(long long id)
        :NetworkResponse(id)
    {
        this->errorCode = 0;
        this->networkErrorCode = 0;
    }

    QList<FileItem> HttpResponse::parseList()
    {
        return QList<FileItem>();
    }

    void HttpResponse::parse()
    {
        //qDebug()<<"header:"<<header;
        if(this->errorCode==0){
            QStringList lines = header.split("\r\n");
            if(lines.size()>0){
                QString line = lines[0];
                int pos = line.indexOf(" ");
                if(pos>-1){
                    this->networkErrorCode = line.mid(pos+1,3).toInt();
                    this->networkErrorMsg = line.mid(pos+5);
                }
                /*QStringList status = lines[0].split(" ");
                if(status.size()>1){
                    this->networkErrorCode = status[1].toInt();
                    //this->networkErrorMsg = lines[0].mid(lines[0].i)
                }*/
            }
        }
    }

    bool HttpResponse::status()
    {
        if(this->errorCode==0){
            if(this->networkErrorCode>=200 &&this->networkErrorCode < 300){
                return true;
            }else{
                return false;
            }
        }else{
            return false;
        }
    }

}
