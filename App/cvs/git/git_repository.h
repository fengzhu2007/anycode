#ifndef GIT_REPOSITORY_H
#define GIT_REPOSITORY_H
#include "cvs/repository.h"
struct git_repository;
struct git_revwalk;
namespace ady {
namespace cvs {
class GitRepositoryPrivate;
    class GitRepository : public Repository
    {
    public:
        GitRepository();
        virtual ~GitRepository();
        virtual void init(const QString& path) override;
        virtual QString path() override;
        virtual QList<Branch> branchLists() override;
        virtual const QString headBranch() override;
        virtual void freeRevwalk() override;
        virtual QList<Commit> commitLists(int num) override;
        virtual QList<Commit>* queryCommit(int num) override;
        virtual QList<DiffFile> diffFileLists(QString oid1=QString(),QString oid2=QString()) override;
        virtual QList<DiffFile>* queryDiff(QString oid1=QString(),QString oid2=QString()) override;
        virtual QList<DiffFile>* statusLists() override;
        virtual Error error() const override;
    private:

        void formatDiffLists(QList<DiffFile>& lists);

        //friend Repository;
    private:
        GitRepositoryPrivate* d;


    };
}
}



#endif // GIT_REPOSITORY_H
