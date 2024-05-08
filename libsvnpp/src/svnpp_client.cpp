#include <svn_pools.h>
#include <svn_hash.h>
#include <svn_path.h>
#include <apr_xlate.h>
#include <svn_cmdline.h>
#include <private/svn_subr_private.h>

#include "svnpp_client.h"
#include "svnpp_callback.h"
#include "error.h"
#include "client/info.h"
#include "client/log_entry.h"
#include "client/diff_summary.h"
#include <vector>
#include <iostream>

#include <QDebug>

namespace ac {
namespace svn {

class SvnppClientPrivate{
public:
    Info info;
    apr_pool_t *pool;
    svn_client_ctx_t *ctx;
    bool online;
    apr_hash_t *changelists;

public:
    SvnppClientPrivate(){
        online = false;
        if (svn_cmdline_init("svn", stderr) == EXIT_SUCCESS){

            pool = apr_allocator_owner_get(svn_pool_create_allocator(FALSE));


            #if defined(WIN32) || defined(__CYGWIN__)
              /* Set the working copy administrative directory name. */
              if (getenv("SVN_ASP_DOT_NET_HACK"))
                {
                  SVNPP_ERR(svn_wc_set_adm_dir("_svn", pool));
                }
            #endif


            SVNPP_ERR(svn_ra_initialize(pool));

            /* Init our changelists hash. */
             changelists = apr_hash_make(pool);

            apr_hash_t *cfg_hash;
            SVNPP_ERR(svn_config__get_default_config(&cfg_hash, pool));
            SVNPP_ERR(svn_client_create_context2(&ctx, cfg_hash, pool));
        }


    }

    ~SvnppClientPrivate(){

    }

};

SvnppClient::SvnppClient(std::string path)
{
    pri = new SvnppClientPrivate;
    pri->info.setPath(path);
    this->init(path);
}

SvnppClient::~SvnppClient(){
    delete pri;
}


bool SvnppClient::online()
{
    return pri->online;
}

void SvnppClient::setOnline(bool online)
{
    pri->online = online;
}

int SvnppClient::init(std::string path)
{

    apr_pool_t *subpool = svn_pool_create(pri->pool);
    svn_pool_clear(subpool);

    svn_opt_revision_t peg_revision;
    peg_revision.kind = svn_opt_revision_unspecified;

    apr_array_header_t* revision_ranges = apr_array_make(pri->pool, 1, sizeof(svn_opt_revision_range_t *));

     apr_hash_t *changelists;
     apr_array_header_t *changelists1;
     changelists = apr_hash_make(pri->pool);
     SVNPP_ERR(svn_hash_keys(&(changelists1), changelists, pri->pool));

    svn_opt_revision_range_t *range = (svn_opt_revision_range_t *)apr_palloc(pri->pool, sizeof(svn_opt_revision_range_t));
    range->start.kind = svn_opt_revision_unspecified;
    range->end.kind = svn_opt_revision_unspecified;
    APR_ARRAY_PUSH(revision_ranges,svn_opt_revision_range_t *) = range;

    svn_client_info_receiver2_t receiver = print_info;
    qDebug()<<"init";
    svn_error_t* err = svn_client_info4(path.c_str(),&peg_revision, &(range->start),svn_depth_empty,TRUE /* fetch_excluded */,TRUE /* fetch_actual_only */,FALSE,changelists1,receiver, this->info(),pri->ctx, subpool);

    svn_pool_destroy(subpool);

    return err!=nullptr?err->apr_err:0;
}

Info* SvnppClient::info()
{
    return &pri->info;
}

std::vector<LogEntry*> SvnppClient::logs(revnum_t from,unsigned int limit)
{
    std::vector<LogEntry*> lists;
    if(from==0){
        return lists;
    }
    //svn_error_t *err;
    //apr_array_header_t *received_opts;
    //received_opts = apr_array_make(pri->pool, SVN_OPT_MAX_OPTIONS, sizeof(int));
    Info* info = this->info();
    std::string url_or_path = (pri->online?info->url():info->path());
    const char *target = url_or_path.c_str();

    apr_array_header_t *targets;
    targets = apr_array_make(pri->pool, 1, sizeof(const char *));
    APR_ARRAY_PUSH(targets, const char *) = target;

    svn_opt_revision_t peg_revision;
    peg_revision.kind = (svn_path_is_url(target) ? svn_opt_revision_head : svn_opt_revision_working);
    //apr_array_header_t *revprops;
    apr_array_header_t* revision_ranges = apr_array_make(pri->pool, 1, sizeof(svn_opt_revision_range_t *));
    //APR_ARRAY_PUSH(revision_ranges,svn_opt_revision_range_t *) = {svn_opt_revision_head};
    svn_opt_revision_range_t *range = (svn_opt_revision_range_t *)apr_palloc(pri->pool, sizeof(svn_opt_revision_range_t));
    if(from==-1){
        range->start.kind = svn_opt_revision_unspecified;
    }else{
        range->start.kind = svn_opt_revision_number;
        range->start.value.number = from;
    }
    range->end.kind = svn_opt_revision_unspecified;
    APR_ARRAY_PUSH(revision_ranges,svn_opt_revision_range_t *) = range;
    SVNPP_ERR(svn_client_log5(targets, &peg_revision,revision_ranges,limit,FALSE,TRUE,TRUE,NULL,svn_log_entry_receiver,&lists,pri->ctx,pri->pool));
    return lists;
}

std::vector<DiffSummary*> SvnppClient::diff(revnum_t start,revnum_t end)
{
     apr_array_header_t *diff_options;
     diff_options = NULL;

    Info* info = this->info();


    //const char *target = url_or_path.c_str();
    const char *target;

    svn_opt_revision_t start_revision;
    svn_opt_revision_t end_revision;
    if(start==0 && end==0){
        start_revision.kind = svn_opt_revision_base;
        end_revision.kind = svn_opt_revision_working;
        target = info->path().c_str();

    }else{
        start_revision.kind = svn_opt_revision_number;
        start_revision.value.number = start;
        end_revision.kind = svn_opt_revision_number;
        end_revision.value.number = end;
        std::string url_or_path = (pri->online?info->url():info->path());
        target = url_or_path.c_str();
    }

    /* First check for a peg revision. */
    svn_opt_revision_t peg_revision;
    //SVNPP_ERR(svn_opt_parse_path(&peg_revision, &truepath, target,iterpool));

    peg_revision.kind = (svn_path_is_url(target) ? svn_opt_revision_head : svn_opt_revision_working);

    svn_client_diff_summarize_func_t func = svn_diff_summary;
    DiffSummaryBaton b;
    b.anchor = "";

    apr_pool_t* iterpool = svn_pool_create(pri->pool);
    svn_pool_clear(iterpool);


    apr_hash_t *changelists;
    apr_array_header_t *changelists1;
    changelists = apr_hash_make(pri->pool);
    SVNPP_ERR(svn_hash_keys(&(changelists1), changelists, pri->pool));


    if(start==0 && end==0){

          SVNPP_ERR(svn_client_diff_summarize2(
                        info->path().c_str(),
                        &start_revision,
                        info->path().c_str(),
                        &end_revision,
                        svn_depth_unknown ,
                        FALSE,
                        changelists1,
                        func, &b,
                        pri->ctx, iterpool));

          qDebug()<<"target:"<<target<<";size:"<<b.lists.size()<<";pp:"<<info->path().c_str();
    }else{
        peg_revision.kind = svn_opt_revision_head;
        SVNPP_ERR(svn_client_diff_summarize_peg2(
                                        info->url().c_str(),
                                        &peg_revision,
                                        &start_revision,
                                        &end_revision,
                                        svn_depth_unknown ,
                                        FALSE,
                                        changelists1,
                                        func, &b,
                                        pri->ctx, iterpool));

        qDebug()<<"url target:"<<target<<";size:"<<b.lists.size()<<";pp:"<<info->url().c_str()<<";start:"<<start_revision.value.number;
    }



    svn_pool_destroy(iterpool);

    return b.lists;
}


std::vector<DiffSummary*> SvnppClient::status()
{
    svn_revnum_t repos_rev = SVN_INVALID_REVNUM;
    svn_opt_revision_t rev;
    const char *target = ".";

    apr_hash_t *changelists;
    apr_array_header_t *changelists1;
    changelists = apr_hash_make(pri->pool);
    SVNPP_ERR(svn_hash_keys(&(changelists1), changelists, pri->pool));


    Info* info = this->info();
    //target = info->path().c_str();

    DiffSummaryBaton b;
    b.anchor = info->path().c_str();

    apr_pool_t* iterpool = svn_pool_create(pri->pool);
    svn_pool_clear(iterpool);

    rev.kind = svn_opt_revision_head;
    svn_error_t* error = svn_client_status6(&repos_rev, pri->ctx, info->path().c_str(), &rev,
                                                 svn_depth_unknown,
                                                 FALSE,
                                                 TRUE,
                                                 TRUE /* check_working_copy */,
                                                 FALSE,
                                                 FALSE,
                                                 TRUE /* depth_as_sticky */,
                                                 changelists1,
                                                 print_status, &b,
                                                 iterpool);

    svn_pool_destroy(iterpool);
    qDebug()<<"svn status";
    qDebug()<<"path:"<<target;
    if(error!=NULL){
        qDebug()<<"error:"<<error->apr_err<<";msg:"<<error->message;
    }

    return b.lists;
}


}
}
