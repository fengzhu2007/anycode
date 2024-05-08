#ifndef SVNPP_CALLBACK_H
#define SVNPP_CALLBACK_H
#include <svn_client.h>
#include "types.h"

svn_error_t * print_info(void *baton,const char *target,const svn_client_info2_t *info,apr_pool_t *pool);
svn_error_t * svn_log_entry_receiver(void *baton,svn_log_entry_t *log_entry,apr_pool_t *pool);

svn_error_t * svn_diff_summary(const svn_client_diff_summarize_t *summary,void *baton,apr_pool_t *pool);

svn_error_t * print_status(void *baton,const char *path,const svn_client_status_t *status,apr_pool_t *pool);
#endif // SVNPP_CALLBACK_H
