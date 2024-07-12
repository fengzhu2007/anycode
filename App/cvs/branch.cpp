#include "branch.h"
namespace ady {
namespace cvs {
    Branch::Branch(QString name,bool head)
    {
        m_name = name;
        m_head = head;
    }
}
}
