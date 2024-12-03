#ifndef SVN_REPOSITORY_H
#define SVN_REPOSITORY_H

#include "cvs/repository.h"


#include <svnpp_client.h>


namespace ady {
namespace cvs {

class SvnRepositoryPrivate;
class SvnRepository : public Repository
{
public:
    SvnRepository();
    virtual ~SvnRepository();
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
    void parseError(const std::pair<int,std::string>& error);

private:
    SvnRepositoryPrivate* d;

};


}
}



//

#endif // SVN_REPOSITORY_H
