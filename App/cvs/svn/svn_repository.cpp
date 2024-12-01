#include "svn_repository.h"
#include "../repository.h"


#include <client/info.h>
#include <client/log_entry.h>
#include <client/diff_summary.h>
#include <QDebug>
#include <QString>
#include <QFileInfo>
using namespace ac::svn;

namespace ady {
namespace cvs {
class SvnRepositoryPrivate{
public:
    ac::svn::SvnppClient* client;
    revnum_t rev = -1;
    Error error;

};

   SvnRepository::SvnRepository()
       :Repository()
   {
        d = new SvnRepositoryPrivate;
        d->client = nullptr;
   }

   SvnRepository::~SvnRepository()
   {
        if(d->client!=nullptr){
            delete d->client;
        }
        delete d;
   }

   void SvnRepository::init(const QString& path)
   {
        d->client = new SvnppClient(path.toStdString());
        ac::svn::Info* info = d->client->info();
        //std::pair<int,std::string> error = d->client->error();
        this->parseError(d->client->error());
   }

   QString SvnRepository::path(){
        if(d->client!=nullptr){
            return QString::fromUtf8(d->client->info()->path().c_str());
        }else{
            return QString();
        }
   }



   QList<Branch> SvnRepository::branchLists(){
       return {};
   }

   const QString SvnRepository::headBranch(){
       return {};
   }

   void SvnRepository::freeRevwalk(){
       d->rev = -1;
   }

   QList<Commit> SvnRepository::commitLists(int num)
   {
       QList<Commit> commits;
       if(d->client!=nullptr){
           d->client->setOnline(true);
           std::vector<LogEntry*> lists = d->client->logs(d->rev,num);
           for(auto item:lists){
               Commit commit;
               commit.setOid(QString("%1").arg(item->revision()));
               commit.setAuthor(QString::fromUtf8(item->author().c_str()));
               commit.setContent(QString::fromUtf8(item->message().c_str()).replace("\n"," "));
               commit.setTime(QDateTime::fromString(QString::fromUtf8(item->date().c_str()),Qt::ISODate));
               commits.push_back(commit);
               d->rev = item->revision();
           }
           qDeleteAll(lists);
           this->parseError(d->client->error());
       }
       return commits;
   }

   QList<Commit>* SvnRepository::queryCommit(int num){
       auto list = new QList<Commit>;
       if(d->client!=nullptr){
           d->client->setOnline(true);
           std::vector<LogEntry*> lists = d->client->logs(d->rev,num);
           for(auto item:lists){
               Commit commit;
               commit.setOid(QString("%1").arg(item->revision()));
               commit.setAuthor(QString::fromUtf8(item->author().c_str()));
               commit.setContent(QString::fromUtf8(item->message().c_str()).replace("\n"," "));
               commit.setTime(QDateTime::fromString(QString::fromUtf8(item->date().c_str()),Qt::ISODate));
               list->push_back(commit);
               d->rev = item->revision();
           }
           qDeleteAll(lists);
           this->parseError(d->client->error());
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
       d->client->setOnline(true);

       std::vector<ac::svn::DiffSummary*> lists;
       // qDebug()<<"start,end:"<<start<<";"<<end;
        if(start==0 && end==0){
            lists = d->client->status();
       }else{
            lists = d->client->diff(start,end);
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

        this->parseError(d->client->error());
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
        d->client->setOnline(true);
        std::vector<ac::svn::DiffSummary*> lists;
        if(start==0 && end==0){
           lists = d->client->status();
        }else{
           lists = d->client->diff(start,end);
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
        this->parseError(d->client->error());
        qDeleteAll(lists);
        this->formatDiffLists(*list);
        return list;
   }

   QList<DiffFile>* SvnRepository::statusLists(){
        auto list = new QList<DiffFile>;



       return list;
   }

   Error SvnRepository::error() const{
       return d->error;
   }

   void SvnRepository::formatDiffLists(QList<DiffFile>& lists)
   {
       QString path = QString::fromUtf8(d->client->info()->path().c_str());
       QList<DiffFile>::iterator iter = lists.begin();
       while(iter!=lists.end()){
           QFileInfo fi(path + "/"+(*iter).path());
           if(fi.exists()){
               (*iter).setFilesize(fi.size());
               (*iter).setFiletime(fi.fileTime(QFile::FileTime::FileModificationTime));
           }
           iter++;
       }
   }

   void SvnRepository::parseError(const std::pair<int,std::string>& error){
       d->error.code = error.first;
       d->error.message = QString::fromUtf8(error.second.c_str());
   }

}
}


