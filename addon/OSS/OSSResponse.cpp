#include "OSSResponse.h"
#include <QXmlStreamReader>
#include <QDateTime>

#include <QDebug>
namespace ady {


    OSSResponse::OSSResponse()
        :HttpResponse()
    {

    }


    QList<FileItem> OSSResponse::parseList()
    {
        QList<FileItem> lists;
        if(body.isEmpty()==false){
            QXmlStreamReader xml(body);
            //bool listFlag = false;
            bool dirItemOpen = false;
            bool fileItemOpen = false;

            FileItem item;
            while (!xml.atEnd()) {
                    QXmlStreamReader::TokenType type = xml.readNext();
                    if(type==QXmlStreamReader::StartElement){
                        if(xml.name()=="CommonPrefixes"){
                            dirItemOpen = true;
                            item.modifyTime = "";
                            item.size = 0;
                        }else if(xml.name()=="Contents"){
                            fileItemOpen = true;
                            item.modifyTime = "";
                            item.size = 0;
                        }
                        if(dirItemOpen==false && xml.name()=="Prefix"){
                            QString dir = xml.readElementText();
                            if(!dir.startsWith("/")){
                                dir = "/" + dir;
                            }
                             this->params["dir"] =dir;
                        }else if(dirItemOpen==true && xml.name()=="Prefix"){
                            QString dir = xml.readElementText();
                            if(!dir.startsWith("/")){
                                dir = "/" + dir;
                            }
                            item.path = dir;
                            item.type = FileItem::Dir;
                            if(item.type==FileItem::Dir){
                                item.name = item.path;
                                if(item.name.endsWith("/")){
                                    item.name = item.name.left(item.name.length() - 1);
                                    item.name = item.name.mid(item.name.lastIndexOf("/")+1);
                                }
                                if(!item.path.endsWith("/")){
                                    item.path += "/";
                                }
                            }
                        }else if(fileItemOpen==true){
                            if(xml.name()=="Key"){
                                QString dir = xml.readElementText();
                                if(!dir.startsWith("/")){
                                    dir = "/" + dir;
                                }
                                item.path = dir;
                                item.type = FileItem::File;
                                item.name = item.path;
                                item.name = item.name.mid(item.name.lastIndexOf("/")+1);
                            }else if(xml.name()=="LastModified"){
                                QDateTime time = QDateTime::fromString(xml.readElementText(),Qt::ISODate);
                                item.modifyTime = time.toString("yyyy-MM-dd HH:mm");
                            }else if(xml.name()=="Size"){
                                item.size = xml.readElementText().toInt();
                            }
                        }

                    }else if(type==QXmlStreamReader::EndElement){
                        if(xml.name()=="CommonPrefixes"){
                            dirItemOpen = false;
                            lists.push_back(item);
                        }else if(xml.name()=="Contents"){
                            fileItemOpen = false;
                            if(item.path.endsWith("/")==false){
                                lists.push_back(item);
                            }

                        }
                    }
              }
              if (xml.hasError()) {

              }
        }

        return lists;
    }

    /*void OSSResponse::parse()
    {
        if(!this->header.isEmpty()){
            this->headers = this->header.split("\r\n");
            int size = this->headers.size();
            if(size>1){
                int pos = this->headers[0].indexOf(" ");
                if(pos>-1){
                    this->networkErrorCode = this->headers[0].mid(pos+1,3).toInt();
                    this->networkErrorMsg = this->headers[0].mid(this->headers[0].indexOf(" ",pos+1));
                }
                QStringList lists = this->headers[0].split(" ");
                if(lists.size()==3){
                    this->networkErrorCode = this->headers[size - 2].left(pos).toInt();
                }
                int pos = this->headers[size - 2].indexOf(" ");
                if(pos>-1){
                    this->networkErrorCode = this->headers[size - 2].left(pos).toInt();
                    this->networkErrorMsg = this->headers[size - 2].right(pos+1);
                }
            }
        }
    }*/

    bool OSSResponse::status()
    {
        return errorCode==0 && (networkErrorCode>=200 && networkErrorCode<300);
    }

}
