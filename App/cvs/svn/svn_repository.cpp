#include "svn_repository.h"
#include "../repository.h"
#ifndef Q_OS_MAC


#include <client/info.h>
#include <client/log_entry.h>
#include <client/diff_summary.h>
#include <QDebug>
#include <QString>
#include <QFileInfo>
using namespace ac::svn;

namespace ady {
namespace cvs {
   SvnRepository::SvnRepository()
       :Repository()
   {
        mClient = nullptr;
   }

   SvnRepository::~SvnRepository()
   {

   }


   void SvnRepository::init(QString path)
   {
        mClient = new SvnppClient(path.toStdString());
        ac::svn::Info* info = mClient->info();
        qDebug()<<info->toString().c_str();
        //qDebug()<<QString(info->toString());

   }

   void SvnRepository::destory()
   {
       if(mClient!=nullptr){
           delete mClient;
       }
   }

   void SvnRepository::freeRevwalk(){
        mCurrentRev = -1;
   }

   QList<Commit> SvnRepository::commitLists(int num)
   {
       QList<Commit> commits;
       if(mClient!=nullptr){
           mClient->setOnline(true);
           std::vector<LogEntry*> lists = mClient->logs(mCurrentRev,num);
           for(auto item:lists){
               Commit commit;
               commit.setOid(QString("%1").arg(item->revision()));
               commit.setAuthor(QString::fromUtf8(item->author().c_str()));
               commit.setContent(QString::fromUtf8(item->message().c_str()).replace("\n"," "));
               commit.setTime(QDateTime::fromString(QString::fromUtf8(item->date().c_str()),Qt::ISODate));
               commits.push_back(commit);
               mCurrentRev = item->revision();
           }
           qDeleteAll(lists);
       }
       return commits;
   }

   QList<Commit>* SvnRepository::queryCommit(int num){
       auto list = new QList<Commit>;
       if(mClient!=nullptr){
           mClient->setOnline(true);
           std::vector<LogEntry*> lists = mClient->logs(mCurrentRev,num);
           for(auto item:lists){
               Commit commit;
               commit.setOid(QString("%1").arg(item->revision()));
               commit.setAuthor(QString::fromUtf8(item->author().c_str()));
               commit.setContent(QString::fromUtf8(item->message().c_str()).replace("\n"," "));
               commit.setTime(QDateTime::fromString(QString::fromUtf8(item->date().c_str()),Qt::ISODate));
               list->push_back(commit);
               mCurrentRev = item->revision();
           }
           qDeleteAll(lists);
       }
       return list;
   }




   QList<DiffFile> SvnRepository::diffFileLists(QString oid1,QString oid2)
   {
       revnum_t start = oid1.toInt();
       revnum_t end = oid2.toInt();

       QList<DiffFile> difffiles;
       if(start>0 || end>0){
           if(end==0){
                end = start;
                start = end - 1;
           }
        }
       mClient->setOnline(true);

       std::vector<ac::svn::DiffSummary*> lists;
       // qDebug()<<"start,end:"<<start<<";"<<end;
        if(start==0 && end==0){
            lists = mClient->status();
       }else{
            lists = mClient->diff(start,end);
        }

       for(auto item:lists){
           DiffFile::Status status = DiffFile::Normal;
           if(item->status()==2){
               status = DiffFile::Change;
           }else if(item->status()==3){
               status = DiffFile::Deletion;
           }else{
               status = (DiffFile::Status)item->status();
           }
           QString path = item->path().c_str();
           path = path.replace("\\","/");
           DiffFile diffFile(path,status);
           difffiles.push_back(diffFile);
       }

        qDeleteAll(lists);


        this->formatDiffLists(difffiles);
        return difffiles;
   }

   QList<DiffFile>* SvnRepository::queryDiff(QString oid1,QString oid2){
        revnum_t start = oid1.toInt();
        revnum_t end = oid2.toInt();
        auto list = new QList<DiffFile>;
        if(start>0 || end>0){
           if(end==0){
               end = start;
               start = end - 1;
           }
        }
        mClient->setOnline(true);
        std::vector<ac::svn::DiffSummary*> lists;
        if(start==0 && end==0){
           lists = mClient->status();
        }else{
           lists = mClient->diff(start,end);
        }
        for(auto item:lists){
           DiffFile::Status status = DiffFile::Normal;
           if(item->status()==2){
               status = DiffFile::Change;
           }else if(item->status()==3){
               status = DiffFile::Deletion;
           }else{
               status = (DiffFile::Status)item->status();
           }
           QString path = item->path().c_str();
           path = path.replace("\\","/");
           DiffFile diffFile(path,status);
           list->push_back(diffFile);
        }

        qDeleteAll(lists);
        this->formatDiffLists(*list);
        return list;
   }

   QList<DiffFile>* SvnRepository::statusLists(){
        auto list = new QList<DiffFile>;



       return list;
   }

   void SvnRepository::formatDiffLists(QList<DiffFile>& lists)
   {
       QString path = QString::fromUtf8(mClient->info()->path().c_str());
       QList<DiffFile>::iterator iter = lists.begin();
       while(iter!=lists.end()){
           QFileInfo fi(path + "/"+(*iter).path());
           if(fi.exists()){
               (*iter).setFilesize(fi.size());
               (*iter).setFiletime(fi.fileTime(QFile::FileTime::FileModificationTime).toString("yyyy-MM-dd HH:mm"));
           }
           iter++;
       }
   }

}
}


#endif
