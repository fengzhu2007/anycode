#include "s3_response.h"
#include <QDebug>
#include <QXmlStreamReader>
#include <QDateTime>
namespace ady {
    S3Response::S3Response(long long id)
        :HttpResponse(id)
    {

    }

    QList<FileItem> S3Response::parseList()
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
                    }else if(xml.name()=="ContinuationToken"){
                        m_continuation_token = xml.readElementText();
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



    bool S3Response::status(){
       return errorCode==0 && (networkErrorCode>=200 && networkErrorCode<300);
    }
}
