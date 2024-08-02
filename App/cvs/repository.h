#ifndef REPOSITORY_H
#define REPOSITORY_H
#include "global.h"
#include <QList>
#include <QString>
#include "commit.h"
#include "diff_file.h"
#include "branch.h"
#include "error.h"
namespace ady {
namespace cvs {
    class Branch;
    class RepositoryPrivate;
    class ANYENGINE_EXPORT Repository
    {
    public:
        template<class T>
        static Repository* getInstance()
        {
            return new T();
        }
        Repository();
        virtual ~Repository();
        virtual void init(const QString& path)=0;
        virtual QString path()=0;
        virtual QList<Commit> commitLists(int num)=0;
        virtual QList<Commit>* queryCommit(int num)=0;
        virtual QList<DiffFile> diffFileLists(QString oid1=QString(),QString oid2=QString())=0;
        virtual QList<DiffFile>* queryDiff(QString oid1=QString(),QString oid2=QString())=0;
        virtual QList<DiffFile>* statusLists()=0;
        virtual void freeRevwalk();
        virtual QList<Branch> branchLists()=0;
        virtual const QString headBranch()=0;
        virtual Error error() const=0;

        //virtual bool switchBranch(const QString& name);
        //virtual bool switchBranch(Branch* branch);
        int rid();


    private:
        //static Repository* instance;
        RepositoryPrivate* d;
        static int count;


    /*protected:
        QList<Branch*> m_branchLists;*/


    };



}
}
#endif // REPOSITORY_H
