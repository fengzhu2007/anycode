#ifndef SVNREPOSITORY_H
#define SVNREPOSITORY_H

#include "cvs/Repository.h"

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
    virtual QList<DiffFile> diffFileLists(QString oid1=QString(),QString oid2=QString()) override;
    virtual QList<DiffFile> statusLists() override;

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

#endif // SVNREPOSITORY_H
