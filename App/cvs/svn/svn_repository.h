#ifndef SVN_REPOSITORY_H
#define SVN_REPOSITORY_H

#include "cvs/repository.h"

#ifndef Q_OS_MAC

#include <svnpp_client.h>


namespace ady {
namespace cvs {

class SvnRepository : public Repository
{
public:
    SvnRepository();
    ~SvnRepository();
    virtual void init(QString path) override;
    virtual void destory() override;
    virtual void freeRevwalk() override;
    virtual QList<Commit> commitLists(int num) override;
    virtual QList<Commit>* queryCommit(int num) override;
    virtual QList<DiffFile> diffFileLists(QString oid1=QString(),QString oid2=QString()) override;
    virtual QList<DiffFile>* queryDiff(QString oid1=QString(),QString oid2=QString()) override;
    virtual QList<DiffFile>* statusLists() override;

private:
    void formatDiffLists(QList<DiffFile>& lists);

private:
    ac::svn::SvnppClient* mClient;
    revnum_t mCurrentRev = -1;
};


}
}

#endif

//

#endif // SVN_REPOSITORY_H
