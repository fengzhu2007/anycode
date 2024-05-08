#include "COSResponse.h"
#include <QDebug>
#include <QXmlStreamReader>
#include <QDateTime>
/*
<?xml version='1.0' encoding='utf-8' ?>
    <ListBucketResult>
        <Name>qinnashou-1251912395</Name>
        <Prefix/>
        <Marker/>
        <MaxKeys>1000</MaxKeys>
        <IsTruncated>false</IsTruncated>
        <Contents>
            <Key>recording/</Key>
            <LastModified>2021-02-22T01:47:34.000Z</LastModified>
            <ETag>&quot;d41d8cd98f00b204e9800998ecf8427e&quot;</ETag>
            <Size>0</Size>
            <Owner>
                <ID>1251912395</ID>
                <DisplayName>1251912395</DisplayName>
            </Owner>
            <StorageClass>STANDARD</StorageClass>
        </Contents>
        <Contents>
            <Key>recording/tiw.txt</Key>
            <LastModified>2021-02-22T01:48:08.000Z</LastModified>
            <ETag>&quot;23d6d45f00b5afd778deed9c328e078e&quot;</ETag>
            <Size>321</Size>
            <Owner>
                <ID>1251912395</ID>
            <DisplayName>1251912395</DisplayName>
            </Owner>
            <StorageClass>STANDARD</StorageClass>
        </Contents>
        <Contents>
            <Key>tiw.txt</Key>
            <LastModified>2021-02-22T01:47:05.000Z</LastModified>
            <ETag>&quot;23d6d45f00b5afd778deed9c328e078e&quot;</ETag>
            <Size>321</Size>
            <Owner>
                <ID>1251912395</ID>
                <DisplayName>1251912395</DisplayName>
            </Owner>
            <StorageClass>STANDARD</StorageClass>
        </Contents>
        <Contents>
            <Key>whiteboard/</Key>
            <LastModified>2021-02-22T01:47:28.000Z</LastModified>
            <ETag>&quot;d41d8cd98f00b204e9800998ecf8427e&quot;</ETag>
            <Size>0</Size>
            <Owner>
                <ID>1251912395</ID>
                <DisplayName>1251912395</DisplayName>
            </Owner>
            <StorageClass>STANDARD</StorageClass>
          </Contents>
          <Contents>
            <Key>whiteboard/tiw.txt</Key>
            <LastModified>2021-02-22T01:47:52.000Z</LastModified>
            <ETag>&quot;23d6d45f00b5afd778deed9c328e078e&quot;</ETag>
            <Size>321</Size>
            <Owner>
                <ID>1251912395</ID>
                <DisplayName>1251912395</DisplayName>
            </Owner>
            <StorageClass>STANDARD</StorageClass>
        </Contents>
        <Contents>
            <Key>whiteboardtiw.txt</Key>
            <LastModified>2021-02-22T01:47:45.000Z</LastModified>
            <ETag>&quot;23d6d45f00b5afd778deed9c328e078e&quot;</ETag>
            <Size>321</Size>
            <Owner>
                <ID>1251912395</ID>
                <DisplayName>1251912395</DisplayName>
            </Owner>
            <StorageClass>STANDARD</StorageClass>
        </Contents>
   </ListBucketResult>*/


namespace ady {
    COSResponse::COSResponse()
        :HttpResponse()
    {

    }


    QList<FileItem> COSResponse::parseList()
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
                        }else if(xml.name()=="NextMarker"){
                            m_marker = xml.readElementText();
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

    /*void COSResponse::parse()
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

    bool COSResponse::status()
    {
        return errorCode==0 && (networkErrorCode>=200 && networkErrorCode<300);
    }
}
