#include "repository.h"
#include "branch.h"
namespace ady {
namespace cvs {


class RepositoryPrivate{
public:
    int count=0;
};

Repository::Repository()
{
    d = new RepositoryPrivate;
    this->used();
}

Repository::~Repository()
{
    delete d;
    qDeleteAll(m_branchLists);
    m_branchLists.clear();
}

void Repository::freeRevwalk()
{

}

QList<Branch*> Repository::branchLists()
{
    return m_branchLists;
}

Branch* Repository::headBranch()
{
    for(auto item:m_branchLists){
        if(item->head()){
            return item;
        }
    }
    return nullptr;
}

bool Repository::switchBranch(const QString& name)
{
    return false;
}

bool Repository::switchBranch(Branch* branch)
{
    return switchBranch(branch->name());
}
void Repository::used(){
    d->count +=1;
}

bool Repository::unUsed(){
    d->count -=1;
    if(d->count<=0){
        delete this;
    }
    return d->count<=0;
}

}
}
