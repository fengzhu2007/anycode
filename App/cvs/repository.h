#ifndef REPOSITORY_H
#define REPOSITORY_H
#include "global.h"
#include <QList>
#include <QString>
#include "commit.h"
#include "diff_file.h"
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
        virtual void destory()=0;
        virtual void init(QString path)=0;
        virtual QList<Commit> commitLists(int num)=0;
        virtual QList<Commit>* queryCommit(int num)=0;
        virtual QList<DiffFile> diffFileLists(QString oid1=QString(),QString oid2=QString())=0;
        virtual QList<DiffFile>* queryDiff(QString oid1=QString(),QString oid2=QString())=0;

        virtual QList<DiffFile>* statusLists()=0;
        virtual void freeRevwalk();


        virtual QList<Branch*> branchLists();
        virtual Branch* headBranch();
        virtual bool switchBranch(const QString& name);
        virtual bool switchBranch(Branch* branch);
        void used();
        bool unUsed();

    private:
        //static Repository* instance;
        RepositoryPrivate* d;


    protected:
        QList<Branch*> m_branchLists;


    };



}
}
#endif // REPOSITORY_H
