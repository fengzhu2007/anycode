#include "{TPL}Response.h"
#include "{TPL}Parse.h"
#include <QDebug>
namespace ady {
    {TPL}Response::{TPL}Response()
        :NetworkResponse()
    {

    }

    QList<FileItem> {TPL}Response::parseList()
    {
        bool mlsd = false;
        if(this->command.toUpper().left(4)=="MLSD"){
            mlsd = true;
        }
        qDebug()<<"body:"<<this->body;
        //qDebug()<<"command:"<<mlsd;
#ifdef Q_OS_MAC
        QStringList lines = this->body.split("\n");
        qDebug()<<"lines:"<<lines;
#else
      QStringList lines = this->body.split("\r\n");
#endif

        QStringList::iterator iter = lines.begin();
        QList<FileItem> lists;
        QString dir = this->params["dir"].toString();
        if(!dir.endsWith("/")){
            dir += "/";
        }
        while(iter!=lines.end()){
            FileItem item;
            {TPL}Parse parse(mlsd);
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

    void {TPL}Response::parse()
    {
        if(!this->header.isEmpty()){
            this->headers = this->header.split("\r\n");
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

    bool {TPL}Response::status(){
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
