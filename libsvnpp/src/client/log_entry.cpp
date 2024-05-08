#include "client/log_entry.h"
#include "svnpp_callback.h"

#include <svn_compat.h>

#include <vector>
#include <iostream>

namespace ac {
namespace svn {
    class LogEntryPrivate{
    public:
        revnum_t rev=0;
        std::string author;
        std::string message;
        std::string date;
    };

    LogEntry::LogEntry(){
        pri = new LogEntryPrivate;
    }

    LogEntry::~LogEntry(){
        delete pri;
    }

    revnum_t LogEntry::revision()
    {
        return pri->rev;
    }

    std::string LogEntry::author()
    {
        return pri->author;
    }

    std::string LogEntry::message()
    {
        return pri->message;
    }

    std::string LogEntry::date()
    {
        return pri->date;
    }



    void LogEntry::setRevision(revnum_t rev)
    {
        pri->rev = rev;
    }

    void LogEntry::setAuthor(std::string author)
    {
        pri->author = author;
    }

    void LogEntry::setMessage(std::string message)
    {
        pri->message = message;
    }

    void LogEntry::setDate(std::string date)
    {
        pri->date = date;
    }





}
}


svn_error_t * svn_log_entry_receiver(void *baton,svn_log_entry_t *log_entry,apr_pool_t *pool)
{
    //svn_cl__log_receiver_baton *lb = (svn_cl__log_receiver_baton *)baton;
    //ac::svn::LogEntryPrivate* pri = (ac::svn::LogEntryPrivate*)baton;
    std::vector<ac::svn::LogEntry*>* lists = (std::vector<ac::svn::LogEntry*>*)baton;

    const char *author;
    const char *date;
    const char *message;

    //if (lb->ctx->cancel_func)
   //   SVN_ERR(lb->ctx->cancel_func(lb->ctx->cancel_baton));

    svn_compat_log_revprops_out(&author, &date, &message, log_entry->revprops);
    /*std::cout<<"author:"<<author<<std::endl;
    std::cout<<"date:"<<date<<std::endl;
    std::cout<<"message:"<<message<<std::endl;
    std::cout.flush();*/
    ac::svn::LogEntry* entry = new ac::svn::LogEntry() ;
    entry->setRevision(log_entry->revision);
    entry->setAuthor(author==NULL?"":author);
    entry->setDate(date==NULL?"":date);
    entry->setMessage(message==NULL?"":message);
    lists->push_back(entry);

    if (log_entry->revision == 0 && message == NULL)
      return SVN_NO_ERROR;


    //diff
    /*if (! SVN_IS_VALID_REVNUM(log_entry->revision))
      {
        if (lb->merge_stack)
          apr_array_pop(lb->merge_stack);

        return SVN_NO_ERROR;
      }*/

    return SVN_NO_ERROR;

}


