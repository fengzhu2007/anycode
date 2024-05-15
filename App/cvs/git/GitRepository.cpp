#include "GitRepository.h"
#include <git2.h>
#include <QString>
#include "cvs/Branch.h"
#include <QDebug>
#include <QByteArray>
#include <QFileInfo>

int diff_output(const git_diff_delta *d, const git_diff_hunk *h, const git_diff_line *l, void *p)
{

    QList<ady::cvs::DiffFile>* payload = (QList<ady::cvs::DiffFile>*)p;
    ady::cvs::DiffFile::Status status = ady::cvs::DiffFile::Normal;
    qDebug()<<"name:"<<QString::fromUtf8(d->new_file.path)<<"status:"<<d->status;
    if(d->status==GIT_DELTA_ADDED){
        status = ady::cvs::DiffFile::Addition;
    }else if(d->status==GIT_DELTA_DELETED){
        status = ady::cvs::DiffFile::Deletion;
    }else if(d->status==GIT_DELTA_MODIFIED){
        status = ady::cvs::DiffFile::Change;
    }else if(d->status==GIT_DELTA_RENAMED){
        status = ady::cvs::DiffFile::Change;
    }
    ady::cvs::DiffFile item(QString::fromUtf8(d->new_file.path),status);

    payload->push_back(item);
    return 0;
}





namespace ady {
namespace cvs {
GitRepository::GitRepository()
    :Repository()
{
    m_repo = nullptr;
    m_revwalk = nullptr;
    git_libgit2_init();

}

GitRepository::~GitRepository()
{

}

void GitRepository::init(QString path)
{
    if(m_repo==nullptr){
        m_path = path;
        //git_libgit2_init();
        /*int result = git_repository_init(&m_repo, path.toLocal8Bit(), 0);
        if(result!=0){
            qDebug()<<"init result 1:"<<result;
        }*/
        int result = git_repository_open_ext(&m_repo, m_path.toLocal8Bit(), 0, NULL);
        if(result!=0){
            qDebug()<<"init result 2:"<<result;
        }
    }
}

void GitRepository::destory()
{
    this->freeRevwalk();
    if(m_repo!=nullptr)
        git_repository_free(this->m_repo);
    git_libgit2_shutdown();
}

QList<Branch*> GitRepository::branchLists()
{
    m_branchLists.clear();
    if(m_repo!=nullptr && m_branchLists.size()==0){
        git_branch_iterator* branch_iterator = nullptr;
        git_reference* tmp_branch = nullptr;
        git_branch_t branch_type;
        git_branch_iterator_new(&branch_iterator, m_repo, GIT_BRANCH_LOCAL);
        while (GIT_ITEROVER != git_branch_next(&tmp_branch, &branch_type, branch_iterator))
        {
            const char* branch_name;
            git_branch_name(&branch_name, tmp_branch);
            bool is_head = git_branch_is_head(tmp_branch);
            Branch* branch = new Branch(QString::fromUtf8(branch_name),is_head);
            m_branchLists.push_back(branch);
        }
    }
    return Repository::branchLists();
}

bool GitRepository::switchBranch(const QString& name)
{
    if(m_repo==nullptr){
        return false;
    }
    git_reference* lookup_branch = nullptr;
    int result = git_branch_lookup(&lookup_branch,m_repo, name.toStdString().c_str()/* "dev"*/, GIT_BRANCH_LOCAL);
    result = git_repository_set_head(m_repo, git_reference_name(lookup_branch));
    return (result == 0);
}

void GitRepository::freeRevwalk()
{
    if(m_revwalk!=nullptr){
        git_revwalk_free(m_revwalk);
        m_revwalk = nullptr;
    }
}

QList<Commit> GitRepository::commitLists(int num)
{
    QList<Commit> lists;
    if(m_repo==nullptr){
        return lists;
    }
    unsigned i = 0;
    int result = 0;
    git_oid oid;
    git_commit *commit = nullptr;
    if(m_revwalk==nullptr){
        result = git_revwalk_new(&m_revwalk, this->m_repo);
        if(result==0){
            result = git_revwalk_sorting(m_revwalk, GIT_SORT_TIME);
            result = git_revwalk_push_head(m_revwalk);
        }else{
            return lists;
        }
    }
    for (; !git_revwalk_next(&oid, m_revwalk); git_commit_free(commit)) {
        if (i++ >= num) {
            break;
        }
        Commit item;
        result = git_commit_lookup(&commit, this->m_repo, &oid);
        if(result!=0){
            break;
        }
        char buf[GIT_OID_HEXSZ + 1];
        git_oid_tostr(buf, sizeof(buf), git_commit_id(commit));
        item.setOid(QString::fromUtf8(buf));
        const git_signature *sig;
        if ((sig = git_commit_author(commit)) != NULL) {
            item.setAuthor(QString::fromUtf8(sig->name));
            item.setEmail(QString::fromUtf8(sig->email));
            item.setTime(QDateTime::fromSecsSinceEpoch(sig->when.time));
        }
        const char *scan, *eol;
        QString content;
        for (scan = git_commit_message(commit); scan && *scan; ) {
              content += QString::fromUtf8(scan);
              break;
        }
        content = content.replace("\n"," ");
        item.setContent(content);
        lists.push_back(item);
    }
    return lists;
}


QList<DiffFile> GitRepository::diffFileLists(QString oid1,QString oid2)
{
    QList<DiffFile> lists;
    git_oid oid;
    git_commit *commit = nullptr;
    git_commit *parent;
    git_tree *a = nullptr;//new tree
    git_tree *b = nullptr;//old tree
    git_diff *diff = nullptr;
    git_diff_options diffopts = GIT_DIFF_OPTIONS_INIT;

    int result = 0;
    if(oid1.isEmpty()){
        //workdir
        /*git_revwalk* revwalk = nullptr;
        result = git_revwalk_new(&revwalk, this->m_repo);
        if(result==0){
            result = git_revwalk_sorting(revwalk, GIT_SORT_TIME);
            result = git_revwalk_push_head(revwalk);
        }else{
            return lists;
        }
        result = git_revwalk_next(&oid, revwalk);
        result = git_commit_lookup(&commit, this->m_repo, &oid);
        result = git_commit_tree(&a, commit);*/
        /*struct diff_options o = {
            GIT_DIFF_OPTIONS_INIT, GIT_DIFF_FIND_OPTIONS_INIT,
            -1, -1, 0, 0, GIT_DIFF_FORMAT_PATCH, NULL, NULL, "."
         };*/
        return statusLists();

        /*result = git_diff_index_to_workdir(&diff,m_repo,nullptr,nullptr);
        git_diff_print(diff, GIT_DIFF_FORMAT_PATCH_HEADER, diff_output, &lists);

        git_diff_free(diff);
        goto final;*/

    }else{
        result = git_oid_fromstr(&oid, oid1.toStdString().c_str());
        if(result==0){
            result = git_commit_lookup(&commit, this->m_repo, &oid);
            if(result==0){
                result = git_commit_tree(&a, commit);
                if(result!=0){
                    goto free_commit;
                }
            }else{
                goto final;
            }
        }else{
            goto final;
        }
        if(oid2.isEmpty()){
            //current commit diff
            int parents = (int)git_commit_parentcount(commit);
            qDebug()<<"parents num:"<<parents;
            if (parents == 1) {
                git_commit_parent(&parent, commit, 0);
                //git_tree *a = nullptr;
                git_commit_tree(&b, parent);
                git_commit_free(parent);
                git_diff_tree_to_tree(&diff, this->m_repo, b, a, &diffopts);
                git_diff_print(diff, GIT_DIFF_FORMAT_NAME_STATUS, diff_output, &lists);
                git_tree_free(b);
            }else{
                for (int i = 0; i < parents; i++) {
                    git_commit_parent(&parent, commit, i);
                    //git_tree *a = nullptr;
                    git_commit_tree(&b, parent);
                    git_commit_free(parent);
                    git_diff_tree_to_tree(&diff, this->m_repo, b, a, &diffopts);
                    git_diff_print(diff, GIT_DIFF_FORMAT_NAME_STATUS, diff_output, &lists);
                    git_tree_free(b);
                }
            }
        }else{
            //diff oid1 and oid2
            result = git_oid_fromstr(&oid, oid2.toStdString().c_str());
            if(result==0){
                result = git_commit_lookup(&commit, this->m_repo, &oid);
                if(result==0){
                    result = git_commit_tree(&b, commit);
                    if(result==0){

                        git_diff_tree_to_tree(&diff, this->m_repo, b, a, &diffopts);
                        git_diff_print(diff, GIT_DIFF_FORMAT_NAME_STATUS, diff_output, &lists);
                        git_tree_free(b);

                    }else{
                        goto free_commit;
                    }
                }else{

                    goto final;
                }

            }else{
                goto final;
            }
        }
    }
    git_tree_free(a);
    free_commit:
        git_commit_free(commit);

    final:
    this->formatDiffLists(lists);
    return lists;
}


QList<DiffFile> GitRepository::statusLists(){

    QList<DiffFile> lists;
    git_status_list *status;
    git_status_options options;
    if (git_repository_is_bare(m_repo)!=0){
        qDebug()<<"Cannot report status on bare repository;"<< git_repository_path(m_repo);
        return lists;
    }

    //char p[] = ".";
    //char * pathspec[8];
    //pathspec[0] = p;

    options.version = GIT_STATUS_OPTIONS_INIT;
    options.show = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
    options.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED  | GIT_STATUS_OPT_SORT_CASE_SENSITIVELY ;
    //options.pathspec.strings  = pathspec;
    //options.pathspec.count = 1;
    int ret = git_status_list_new(&status,this->m_repo,NULL);
    size_t i, maxi;
    const git_status_entry *s;
    if(ret!=0){
        qDebug()<<"error:"<<ret;
        goto result;
    }
    maxi = git_status_list_entrycount(status);
    for (i = 0; i < maxi; ++i) {
        char *istatus = NULL;
        s = git_status_byindex(status, i);
        if (s->status == GIT_STATUS_CURRENT){
            continue;
        }
        ady::cvs::DiffFile::Status ss = ady::cvs::DiffFile::Normal;
        if(s->status&GIT_STATUS_INDEX_NEW){
            ss = ady::cvs::DiffFile::Addition;
        }else if(s->status& GIT_STATUS_INDEX_MODIFIED){
            ss = ady::cvs::DiffFile::Change;
        }else if(s->status& GIT_STATUS_INDEX_DELETED){
            ss = ady::cvs::DiffFile::Deletion;
        }else if(s->status& GIT_STATUS_INDEX_RENAMED){
            ss = ady::cvs::DiffFile::Change;
        }else if(s->status& GIT_STATUS_INDEX_TYPECHANGE){
            ss = ady::cvs::DiffFile::Change;
        }else{
            continue;
        }
        ady::cvs::DiffFile item(QString::fromUtf8(s->head_to_index->new_file.path),ss);
        lists.push_back(item);
    }


    for (i = 0; i < maxi; ++i) {
        char *wstatus = NULL;
        s = git_status_byindex(status, i);
        if (s->status == GIT_STATUS_CURRENT || s->index_to_workdir == NULL)
            continue;

        ady::cvs::DiffFile::Status ss = ady::cvs::DiffFile::Normal;
        if(s->status& GIT_STATUS_WT_MODIFIED){
            ss = ady::cvs::DiffFile::Change;
        }else if(s->status& GIT_STATUS_WT_DELETED){
            ss = ady::cvs::DiffFile::Deletion;
        }else if(s->status& GIT_STATUS_WT_RENAMED){
            ss = ady::cvs::DiffFile::Change;
        }else if(s->status& GIT_STATUS_WT_TYPECHANGE){
            ss = ady::cvs::DiffFile::Change;
        }else{
            continue;
        }
        ady::cvs::DiffFile item(QString::fromUtf8(s->index_to_workdir->new_file.path),ss);
        lists.push_back(item);

    }

    for (i = 0; i < maxi; ++i) {
        s = git_status_byindex(status, i);
        if (s->status == GIT_STATUS_WT_NEW) {
            ady::cvs::DiffFile::Status ss = ady::cvs::DiffFile::Addition;
            ady::cvs::DiffFile item(QString::fromUtf8(s->index_to_workdir->old_file.path),ss);
            lists.push_back(item);
        }
    }

    result:

    git_status_list_free(status);
    qDebug()<<"size:"<<lists.size();
    this->formatDiffLists(lists);
    return lists;
}


void GitRepository::formatDiffLists(QList<DiffFile>& lists)
{
    QList<DiffFile>::iterator iter = lists.begin();
    while(iter!=lists.end()){
        QFileInfo fi(m_path + "/"+(*iter).path());
        if(fi.exists()){
            (*iter).setFilesize(fi.size());
            (*iter).setFiletime(fi.fileTime(QFile::FileTime::FileModificationTime).toString("yyyy-MM-dd HH:mm"));
        }
        iter++;
    }
}


}
}
