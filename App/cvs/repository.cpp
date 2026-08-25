#include "repository.h"
namespace ady {
namespace cvs {

int Repository::count = 0;
class RepositoryPrivate{
public:
    int count=0;
    int rid;
};

Repository::Repository()
{
    d = new RepositoryPrivate;
    d->rid = Repository::count;
}

Repository::~Repository()
{
    delete d;
}

void Repository::freeRevwalk()
{

}

int Repository::rid(){
    return d->rid;
}

DiffContent Repository::diffContent(const QString &filePath, QString oid1, QString oid2)
{
    Q_UNUSED(filePath);
    Q_UNUSED(oid1);
    Q_UNUSED(oid2);
    return DiffContent();
}


}
}
