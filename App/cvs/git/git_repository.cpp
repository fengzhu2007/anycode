#include "git_repository.h"
#include <git2.h>
#include <QString>
#include "cvs/branch.h"
#include "cvs/diff_content.h"
#include <QDebug>
#include <QByteArray>
#include <QFileInfo>

int diff_output(const git_diff_delta *d, const git_diff_hunk *h, const git_diff_line *l, void *p)
{

    QList<ady::cvs::DiffFile>* payload = (QList<ady::cvs::DiffFile>*)p;
    ady::cvs::DiffFile::Status status = ady::cvs::DiffFile::Normal;
    //qDebug()<<"name:"<<QString::fromUtf8(d->new_file.path)<<"status:"<<d->status;
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

class GitRepositoryPrivate{
public:
    git_repository* repo;
    git_revwalk* revwalk;
    QString path;
};



GitRepository::GitRepository()
    :Repository()
{
    d = new GitRepositoryPrivate;
    d->repo = nullptr;
    d->revwalk = nullptr;
    git_libgit2_init();

}

GitRepository::~GitRepository()
{
    this->freeRevwalk();
    if(d->repo!=nullptr)
        git_repository_free(d->repo);
    git_libgit2_shutdown();
    delete d;
}

void GitRepository::init(const QString& path)
{
    if(d->repo==nullptr){
        d->path = path;
        //git_libgit2_init();
        /*int result = git_repository_init(&m_repo, path.toLocal8Bit(), 0);
        if(result!=0){
            qDebug()<<"init result 1:"<<result;
        }*/
        int result = git_repository_open_ext(&d->repo, d->path.toLocal8Bit(), 0, NULL);
        if(result!=0){
            qDebug()<<"init result 2:"<<result;
        }
    }
}

QString GitRepository::path(){
    return d->path;
}


QList<Branch> GitRepository::branchLists(){
    QList<Branch> list;
    if(d->repo!=nullptr){
        git_branch_iterator* branch_iterator = nullptr;
        git_reference* tmp_branch = nullptr;
        git_branch_t branch_type;
        git_branch_iterator_new(&branch_iterator, d->repo, GIT_BRANCH_LOCAL);
        while (GIT_ITEROVER != git_branch_next(&tmp_branch, &branch_type, branch_iterator))
        {
            const char* branch_name;
            git_branch_name(&branch_name, tmp_branch);
            bool is_head = git_branch_is_head(tmp_branch);
            list.push_back(Branch{QString::fromUtf8(branch_name),is_head});
        }
    }
    return list;
}

const QString GitRepository::headBranch(){
    QString head;
    if(d->repo!=nullptr){
        git_branch_iterator* branch_iterator = nullptr;
        git_reference* tmp_branch = nullptr;
        git_branch_t branch_type;
        git_branch_iterator_new(&branch_iterator, d->repo, GIT_BRANCH_LOCAL);
        while (GIT_ITEROVER != git_branch_next(&tmp_branch, &branch_type, branch_iterator))
        {
            const char* branch_name;
            git_branch_name(&branch_name, tmp_branch);
            bool is_head = git_branch_is_head(tmp_branch);
            if(is_head){
                head = QString::fromUtf8(branch_name);
            }
        }
    }
    return head;
}



void GitRepository::freeRevwalk()
{
    if(d->revwalk!=nullptr){
        git_revwalk_free(d->revwalk);
        d->revwalk = nullptr;
    }
}

QList<Commit> GitRepository::commitLists(int num)
{
    QList<Commit> lists;
    if(d->repo==nullptr){
        return lists;
    }
    unsigned i = 0;
    int result = 0;
    git_oid oid;
    git_commit *commit = nullptr;
    if(d->revwalk==nullptr){
        result = git_revwalk_new(&d->revwalk, d->repo);
        if(result==0){
            result = git_revwalk_sorting(d->revwalk, GIT_SORT_TIME);
            result = git_revwalk_push_head(d->revwalk);
        }else{
            return lists;
        }
    }
    for (; !git_revwalk_next(&oid, d->revwalk); git_commit_free(commit)) {
        if (i++ >= num) {
            break;
        }
        Commit item;
        result = git_commit_lookup(&commit, d->repo, &oid);
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

QList<Commit>* GitRepository::queryCommit(int num){
    auto list = new QList<Commit>;
    if(d->repo==nullptr){
        return list;
    }
    unsigned i = 0;
    int result = 0;
    git_oid oid;
    git_commit *commit = nullptr;
    if(d->revwalk==nullptr){
        result = git_revwalk_new(&d->revwalk, d->repo);
        if(result==0){
              result = git_revwalk_sorting(d->revwalk, GIT_SORT_TIME);
              result = git_revwalk_push_head(d->revwalk);
        }else{
              return list;
        }
    }
    for (; !git_revwalk_next(&oid, d->revwalk); git_commit_free(commit)) {
        if (i++ >= num) {
              break;
        }
        Commit item;
        result = git_commit_lookup(&commit, d->repo, &oid);
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
        list->push_back(item);
    }
    return list;
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
        return lists;
    }else{
        result = git_oid_fromstr(&oid, oid1.toStdString().c_str());
        if(result==0){
            result = git_commit_lookup(&commit, d->repo, &oid);
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
            //qDebug()<<"parents num:"<<parents;
            if (parents == 1) {
                git_commit_parent(&parent, commit, 0);
                //git_tree *a = nullptr;
                git_commit_tree(&b, parent);
                git_commit_free(parent);
                git_diff_tree_to_tree(&diff, d->repo, b, a, &diffopts);
                git_diff_print(diff, GIT_DIFF_FORMAT_NAME_STATUS, diff_output, &lists);
                git_tree_free(b);
            }else{
                for (int i = 0; i < parents; i++) {
                    git_commit_parent(&parent, commit, i);
                    //git_tree *a = nullptr;
                    git_commit_tree(&b, parent);
                    git_commit_free(parent);
                    git_diff_tree_to_tree(&diff, d->repo, b, a, &diffopts);
                    git_diff_print(diff, GIT_DIFF_FORMAT_NAME_STATUS, diff_output, &lists);
                    git_tree_free(b);
                }
            }
        }else{
            //diff oid1 and oid2
            result = git_oid_fromstr(&oid, oid2.toStdString().c_str());
            if(result==0){
                result = git_commit_lookup(&commit, d->repo, &oid);
                if(result==0){
                    result = git_commit_tree(&b, commit);
                    if(result==0){

                        git_diff_tree_to_tree(&diff, d->repo, b, a, &diffopts);
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

QList<DiffFile>* GitRepository::queryDiff(QString oid1,QString oid2){

    auto  list = new QList<DiffFile>;
    git_oid oid;
    git_commit *commit = nullptr;
    git_commit *parent;
    git_tree *a = nullptr;//new tree
    git_tree *b = nullptr;//old tree
    git_diff *diff = nullptr;
    git_diff_options diffopts = GIT_DIFF_OPTIONS_INIT;

    int result = 0;
    if(oid1.isEmpty()){
        return statusLists();
    }else{
        result = git_oid_fromstr(&oid, oid1.toStdString().c_str());
        if(result==0){
            result = git_commit_lookup(&commit, d->repo, &oid);
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
            //qDebug()<<"parents num:"<<parents;
            if (parents == 1) {
                git_commit_parent(&parent, commit, 0);
                //git_tree *a = nullptr;
                git_commit_tree(&b, parent);
                git_commit_free(parent);
                git_diff_tree_to_tree(&diff, d->repo, b, a, &diffopts);
                git_diff_print(diff, GIT_DIFF_FORMAT_NAME_STATUS, diff_output, list);
                git_tree_free(b);
            }else{
                for (int i = 0; i < parents; i++) {
                    git_commit_parent(&parent, commit, i);
                    //git_tree *a = nullptr;
                    git_commit_tree(&b, parent);
                    git_commit_free(parent);
                    git_diff_tree_to_tree(&diff, d->repo, b, a, &diffopts);
                    git_diff_print(diff, GIT_DIFF_FORMAT_NAME_STATUS, diff_output, list);
                    git_tree_free(b);
                }
            }
        }else{
            //diff oid1 and oid2
            result = git_oid_fromstr(&oid, oid2.toStdString().c_str());
            if(result==0){
                result = git_commit_lookup(&commit, d->repo, &oid);
                if(result==0){
                    result = git_commit_tree(&b, commit);
                    if(result==0){

                        git_diff_tree_to_tree(&diff, d->repo, b, a, &diffopts);
                        git_diff_print(diff, GIT_DIFF_FORMAT_NAME_STATUS, diff_output, list);
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
    this->formatDiffLists(*list);
    return list;
}


QList<DiffFile>* GitRepository::statusLists(){

    auto list = new QList<DiffFile>;
    git_status_list *status;
    git_status_options options;
    if (git_repository_is_bare(d->repo)!=0){
        //qDebug()<<"Cannot report status on bare repository;"<< git_repository_path(m_repo);
        return list;
    }

    //char p[] = ".";
    //char * pathspec[8];
    //pathspec[0] = p;

    options.version = GIT_STATUS_OPTIONS_INIT;
    options.show = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
    options.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED  | GIT_STATUS_OPT_SORT_CASE_SENSITIVELY ;
    //options.pathspec.strings  = pathspec;
    //options.pathspec.count = 1;
    int ret = git_status_list_new(&status,d->repo,NULL);
    size_t i, maxi;
    const git_status_entry *s;
    if(ret!=0){
        //qDebug()<<"error:"<<ret;
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
        list->push_back(item);
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
        list->push_back(item);

    }

    for (i = 0; i < maxi; ++i) {
        s = git_status_byindex(status, i);
        if (s->status == GIT_STATUS_WT_NEW) {
            ady::cvs::DiffFile::Status ss = ady::cvs::DiffFile::Addition;
            ady::cvs::DiffFile item(QString::fromUtf8(s->index_to_workdir->old_file.path),ss);
            list->push_back(item);
        }
    }

    result:

    git_status_list_free(status);
    this->formatDiffLists(*list);
    return list;
}

Error GitRepository::error() const {
    return {0,{}};
}


void GitRepository::formatDiffLists(QList<DiffFile>& lists)
{
    QList<DiffFile>::iterator iter = lists.begin();
    while(iter!=lists.end()){
        QFileInfo fi(d->path + "/"+(*iter).path());
        if(fi.exists()){
            (*iter).setFilesize(fi.size());
            (*iter).setFiletime(fi.fileTime(QFile::FileTime::FileModificationTime));
        }
        iter++;
    }
}

// Line-level diff callback data structure
struct DiffContentPayload {
    DiffContent *content;
    DiffHunk currentHunk;
    bool inHunk;
};

// Hunk callback: called when a new hunk starts
static int diff_hunk_callback(const git_diff_delta *delta, const git_diff_hunk *hunk, void *payload)
{
    Q_UNUSED(delta);
    DiffContentPayload *p = static_cast<DiffContentPayload*>(payload);

    // Save previous hunk if any
    if (p->inHunk) {
        p->content->addHunk(p->currentHunk);
    }

    // Start new hunk
    p->currentHunk = DiffHunk();
    p->currentHunk.setOldStart(hunk->old_start);
    p->currentHunk.setOldCount(hunk->old_lines);
    p->currentHunk.setNewStart(hunk->new_start);
    p->currentHunk.setNewCount(hunk->new_lines);
    p->currentHunk.setHeader(QString::fromUtf8(hunk->header, hunk->header_len));
    p->inHunk = true;

    return 0;
}

// Line callback: called for each line in a hunk
static int diff_line_callback(const git_diff_delta *delta, const git_diff_hunk *hunk, const git_diff_line *line, void *payload)
{
    Q_UNUSED(delta);
    Q_UNUSED(hunk);
    DiffContentPayload *p = static_cast<DiffContentPayload*>(payload);

    DiffLine::Type type = DiffLine::Context;
    QString content = QString::fromUtf8(line->content, line->content_len);
    // Remove trailing newline
    if (content.endsWith('\n')) {
        content.chop(1);
    }

    int oldLineNo = -1;
    int newLineNo = -1;

    switch (line->origin) {
    case GIT_DIFF_LINE_CONTEXT:
        type = DiffLine::Context;
        oldLineNo = line->old_lineno;
        newLineNo = line->new_lineno;
        break;
    case GIT_DIFF_LINE_ADDITION:
        type = DiffLine::Addition;
        newLineNo = line->new_lineno;
        break;
    case GIT_DIFF_LINE_DELETION:
        type = DiffLine::Deletion;
        oldLineNo = line->old_lineno;
        break;
    case GIT_DIFF_LINE_CONTEXT_EOFNL:
    case GIT_DIFF_LINE_ADD_EOFNL:
    case GIT_DIFF_LINE_DEL_EOFNL:
        // Ignore EOF newline changes
        return 0;
    default:
        // Ignore other line types
        return 0;
    }

    DiffLine diffLine(type, content, oldLineNo, newLineNo);
    p->currentHunk.addLine(diffLine);

    return 0;
}

DiffContent GitRepository::diffContent(const QString &filePath, QString oid1, QString oid2)
{
    DiffContent result;

    if (d->repo == nullptr || oid1.isEmpty()) {
        return result;
    }

    git_oid oid_old, oid_new;
    git_commit *commit_old = nullptr;
    git_commit *commit_new = nullptr;
    git_tree *tree_old = nullptr;
    git_tree *tree_new = nullptr;
    git_blob *blob_old = nullptr;
    git_blob *blob_new = nullptr;
    git_diff *diff = nullptr;
    git_diff_options opts = GIT_DIFF_OPTIONS_INIT;

    int ret = 0;

    // Parse old version commit
    ret = git_oid_fromstr(&oid_old, oid1.toStdString().c_str());
    if (ret != 0) goto cleanup;

    ret = git_commit_lookup(&commit_old, d->repo, &oid_old);
    if (ret != 0) goto cleanup;

    ret = git_commit_tree(&tree_old, commit_old);
    if (ret != 0) goto cleanup;

    // Parse new version commit (if oid2 is empty, use HEAD)
    if (oid2.isEmpty()) {
        // Use working directory comparison
        // Simplified: compare commit with its parent
        int parent_count = git_commit_parentcount(commit_old);
        if (parent_count > 0) {
            git_commit *parent = nullptr;
            ret = git_commit_parent(&parent, commit_old, 0);
            if (ret != 0) {
                git_commit_free(parent);
                goto cleanup;
            }
            ret = git_commit_tree(&tree_new, parent);
            git_commit_free(parent);
            if (ret != 0) goto cleanup;

            // Swap old and new to show changes from parent to commit
            git_tree *temp = tree_old;
            tree_old = tree_new;
            tree_new = temp;
        } else {
            // Initial commit, no parent
            goto cleanup;
        }
    } else {
        ret = git_oid_fromstr(&oid_new, oid2.toStdString().c_str());
        if (ret != 0) goto cleanup;

        ret = git_commit_lookup(&commit_new, d->repo, &oid_new);
        if (ret != 0) goto cleanup;

        ret = git_commit_tree(&tree_new, commit_new);
        if (ret != 0) goto cleanup;
    }

    // Get file entries
    {
        git_tree_entry *entry_old = nullptr;
        git_tree_entry *entry_new = nullptr;
        std::string pathStr = filePath.toStdString();

        // git_tree_entry_bypath returns int, result stored in first parameter
        ret = git_tree_entry_bypath(&entry_old, tree_old, pathStr.c_str());
        if (ret != 0 && ret != GIT_ENOTFOUND) goto cleanup;
        if (ret == GIT_ENOTFOUND) entry_old = nullptr;

        ret = git_tree_entry_bypath(&entry_new, tree_new, pathStr.c_str());
        if (ret != 0 && ret != GIT_ENOTFOUND) {
            if (entry_old) git_tree_entry_free(entry_old);
            goto cleanup;
        }
        if (ret == GIT_ENOTFOUND) entry_new = nullptr;
        ret = 0; // Reset to success

        if (entry_old) {
            ret = git_blob_lookup(&blob_old, d->repo, git_tree_entry_id(entry_old));
            git_tree_entry_free(entry_old);
            if (ret != 0) {
                if (entry_new) git_tree_entry_free(entry_new);
                goto cleanup;
            }
        }

        if (entry_new) {
            ret = git_blob_lookup(&blob_new, d->repo, git_tree_entry_id(entry_new));
            git_tree_entry_free(entry_new);
            if (ret != 0) goto cleanup;
        }
    }

    // Perform line-level diff using git_diff_blobs
    {
        DiffContentPayload payload;
        payload.content = &result;
        payload.inHunk = false;

        ret = git_diff_blobs(blob_old, nullptr, blob_new, nullptr,
                             &opts, nullptr, nullptr, diff_hunk_callback, diff_line_callback, &payload);

        if (payload.inHunk) {
            // Save last hunk
            result.addHunk(payload.currentHunk);
        }
    }

cleanup:
    if (blob_old) git_blob_free(blob_old);
    if (blob_new) git_blob_free(blob_new);
    if (tree_old) git_tree_free(tree_old);
    if (tree_new) git_tree_free(tree_new);
    if (commit_old) git_commit_free(commit_old);
    if (commit_new) git_commit_free(commit_new);
    if (diff) git_diff_free(diff);

    return result;
}


}
}
