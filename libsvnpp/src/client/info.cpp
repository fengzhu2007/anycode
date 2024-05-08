#include "client/info.h"
#include "svnpp_client.h"
#include "svnpp_callback.h"
#include <string>
#include <sstream>
#include <iostream>

namespace ac {
namespace svn {
    class InfoPrivate
    {
    public:
        std::string url;
        std::string root_url;
        std::string path;
        std::string uuid;
        filesize_t filesize;
        revnum_t rev=0;
        revnum_t last_changed_rev=0;
        time_t last_changed_date;
        std::string last_changed_author;


    };

    Info::Info(){
        pri = new InfoPrivate();
    }

    Info::~Info()
    {
        delete pri;
    }

    std::string Info::url()
    {
        return pri->url;
    }

    std::string Info::path()
    {
        return pri->path;
    }

    filesize_t Info::filesize()
    {
        return pri->filesize;
    }

    void Info::setUrl(std::string url)
    {
        pri->url = url;
    }

    void Info::setRootUrl(std::string root_url)
    {
        pri->root_url = root_url;
    }

    void Info::setPath(std::string path)
    {
        pri->path = path;
    }

    void Info::setFilesize(filesize_t filesize)
    {
        pri->filesize = filesize;
    }

    void Info::setUUID(std::string uuid)
    {
        pri->uuid = uuid;
    }

    void Info::setRevision(revnum_t rev)
    {
        pri->rev = rev;
    }

    std::string Info::toString()
    {
        std::ostringstream ostr;
        ostr<<"ac::svn::Info:{url:"<<pri->url<<",path:"<<pri->path<<"}";
        return ostr.str();
    }

}
}

svn_error_t * print_info(void *baton,const char *target,const svn_client_info2_t *info,apr_pool_t *pool)
{
    //(void)target;
    //(void)pool;
     ac::svn::Info* in = (ac::svn::Info*)baton;

     if (info->wc_info && info->wc_info->wcroot_abspath){
        in->setPath(info->wc_info->wcroot_abspath);
     }

     if (info->URL){
         in->setUrl(info->URL);
     }
     if (info->repos_root_URL){
         in->setRootUrl(info->repos_root_URL);
     }

     if (info->repos_UUID){
         in->setUUID(info->repos_UUID);
     }

     in->setFilesize(info->size);

     if (SVN_IS_VALID_REVNUM(info->rev)){
         in->setRevision(info->rev);
     }
     return SVN_NO_ERROR;
}
