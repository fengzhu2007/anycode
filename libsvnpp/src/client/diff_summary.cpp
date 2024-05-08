#include "client/diff_summary.h"
#include "svnpp_callback.h"

#include <svn_compat.h>
#include <svn_path.h>

#include <vector>
#include <iostream>

#include <QDebug>
#include <QFile>

namespace ac {
namespace svn {
class DiffSummaryPrivate{
public:
    int status=0;
    std::string path;
};


DiffSummary::DiffSummary()
{
    pri = new DiffSummaryPrivate;
}

DiffSummary::~DiffSummary()
{
    delete pri;
}

int DiffSummary::status()
{
    return pri->status;
}

std::string DiffSummary::path()
{
    return pri->path;
}


void DiffSummary::setStatus(int status)
{
    pri->status = status;
}

void DiffSummary::setPath(std::string path)
{
    pri->path = path;
}


}

}










svn_error_t * svn_diff_summary(const svn_client_diff_summarize_t *summary,void *baton,apr_pool_t *pool)
{
    ac::svn::DiffSummaryBaton* b = static_cast<ac::svn::DiffSummaryBaton*>(baton);
    const char *path = b->anchor.c_str();

    if (b->igore_prototoies && summary->summarize_kind == svn_client_diff_summarize_kind_normal)
      return SVN_NO_ERROR;

    if(summary->node_kind!=svn_node_file){
        return SVN_NO_ERROR;
    }

    if (svn_path_is_url(path)){
        path = svn_path_url_add_component2(path, summary->path, pool);
    }else{
        path = svn_dirent_join(path, summary->path, pool);
        path = svn_dirent_local_style(path, pool);
    }
    //qDebug()<<"path:"<<summary->path;
    ac::svn::DiffSummary* item = new ac::svn::DiffSummary ();
    item->setPath(path);
    item->setStatus(summary->summarize_kind);
    b->lists.push_back(item);

    return SVN_NO_ERROR;
}


svn_error_t * print_status(void *baton,const char *path,const svn_client_status_t *status,apr_pool_t *pool)
{
    //qDebug()<<"abs path:"<<path;
    ac::svn::DiffSummaryBaton* b = static_cast<ac::svn::DiffSummaryBaton*>(baton);
    //qDebug()<<"anchor:"<<b->anchor.c_str();

    const char *local_abspath = status->local_abspath;
    std::string abspath = status->local_abspath;
    abspath = abspath.substr(b->anchor.length() + 1);

    ac::svn::DiffSummary* item = nullptr;
    //int size = b->anchor.length();

    //path = svn_relpath_join(b->anchor.c_str(), local_abspath, pool);
    //path = svn_dirent_local_style(path, pool);
    //qDebug()<<"path:"<<abspath.c_str();
    if(status->kind == svn_node_file){
        if(status->node_status==svn_wc_status_deleted){
            item = new ac::svn::DiffSummary ();
            item->setPath(abspath.c_str());
            item->setStatus(3);
            //qDebug()<<"delete";
        }else if(status->node_status==svn_wc_status_added){
            item = new ac::svn::DiffSummary ();
            item->setPath(abspath.c_str());
            item->setStatus(1);
            //qDebug()<<"add";
        }else if(status->node_status==svn_wc_status_modified){
            item = new ac::svn::DiffSummary ();
            item->setPath(abspath.c_str());
            item->setStatus(2);
            //qDebug()<<"modify";
        }else if(status->node_status==svn_wc_status_unversioned){
            item = new ac::svn::DiffSummary ();
            item->setPath(abspath.c_str());
            item->setStatus(1);
            //qDebug()<<"unversion";
        }else if(status->node_status==svn_wc_status_missing){
            item = new ac::svn::DiffSummary ();
            item->setPath(abspath.c_str());
            item->setStatus(3);
            //qDebug()<<"mission";
        }

    }else if(status->kind == svn_node_unknown){
        if(status->node_status == svn_wc_status_unversioned){
            if(status->filesize>=0){
                item = new ac::svn::DiffSummary ();
                item->setPath(abspath.c_str());
                item->setStatus(1);
            }
        }
    }
    if(item!=nullptr){
        b->lists.push_back(item);
    }
    /*qDebug()<<"print status local path:"<<local_abspath;
    qDebug()<<" path:"<<path;
    qDebug()<<"status king:"<<status->kind;
    qDebug()<<"status:"<<status->node_status;
    qDebug()<<"filesize:"<<status->filesize;*/
    return SVN_NO_ERROR;
}
