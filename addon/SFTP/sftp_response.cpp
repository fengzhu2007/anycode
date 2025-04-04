#include "sftp_response.h"
#include "sftp_parse.h"
#include <QDebug>
namespace ady {
    SFTPResponse::SFTPResponse(long long id)
        :NetworkResponse(id)
    {

    }

    QList<FileItem> SFTPResponse::parseList()
    {
        //bool mlsd = false;
        //qDebug()<<"body:"<<this->body;
        //qDebug()<<"command:"<<mlsd;
#ifdef Q_OS_MAC
        QStringList lines = this->body.split("\n");
        qDebug()<<"lines:"<<lines;
#else
      QStringList lines = this->body.split("\n");
#endif
        //qDebug()<<"lines :"<<lines.length();
        QStringList::iterator iter = lines.begin();
        QList<FileItem> lists;
        QString dir = this->params["dir"].toString();
        if(!dir.endsWith("/")){
            dir += "/";
        }
        while(iter!=lines.end()){
            FileItem item;
            SFTPParse parse;
            if(parse.lineParse(*iter,item)){
                if(item.name.compare(QString("."))==0 || item.name.compare(QString("..")) == 0){
                    goto gt;
                }
                item.path  = dir + item.name ;
                if(item.type==FileItem::Dir){
                    item.path  += "/";
                }
                lists.push_back(item);
            }
            gt:
            iter++;
        }
        return lists;
    }

    void SFTPResponse::parse()
    {
        if(!this->header.isEmpty()){
            this->headers = this->header.split("\n");
            int size = this->headers.size();
            if(size>1){

				int index = size - 2;
                int pos = this->headers[index].indexOf(" ");
                if(pos>-1){
                    bool ok = false;
                    this->networkErrorCode = this->headers[index].left(pos).toInt(&ok);
                    if(!ok){
                        this->networkErrorCode = 0;
                    }
                    this->networkErrorMsg = this->headers[index].mid(pos+1);
                    //qDebug()<<"command:"<<this->command<<";error:"<<this->errorCode<<";"<<this->networkErrorCode<<";msg:"<<this->networkErrorMsg;
                }
            }
        }
    }

    bool SFTPResponse::status(){
        if(this->errorCode==0){
            if(this->networkErrorCode==0){
                return true;
            }else{
                return false;
            }
        }else{
            return false;
        }
    }
}
