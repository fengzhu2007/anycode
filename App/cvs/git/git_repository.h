#ifndef GIT_REPOSITORY_H
#define GIT_REPOSITORY_H
#include "cvs/repository.h"
struct git_repository;
struct git_revwalk;
namespace ady {
namespace cvs {

    class GitRepository : public Repository
    {
    public:
        ~GitRepository();
        virtual void init(QString path) override;
        virtual void destory() override;
        virtual QList<Branch*> branchLists() override;
        virtual bool switchBranch(const QString& name) override;
        virtual void freeRevwalk() override;
        virtual QList<Commit> commitLists(int num) override;
        virtual QList<Commit>* queryCommit(int num) override;
        virtual QList<DiffFile> diffFileLists(QString oid1=QString(),QString oid2=QString()) override;
        virtual QList<DiffFile>* queryDiff(QString oid1=QString(),QString oid2=QString()) override;
        virtual QList<DiffFile>* statusLists() override;

    private:
        GitRepository();
        void formatDiffLists(QList<DiffFile>& lists);

        friend Repository;
    private:
        git_repository* m_repo;
        git_revwalk* m_revwalk;
        QString m_path;

    };
}
}



#endif // GIT_REPOSITORY_H
