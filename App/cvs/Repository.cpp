#include "Repository.h"
#include "Branch.h"
namespace ady {
namespace cvs {

Repository* Repository::m_instance = nullptr;

Repository::Repository()
{

}

Repository::~Repository()
{
    m_instance = nullptr;
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

}
}
