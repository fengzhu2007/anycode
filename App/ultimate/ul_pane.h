#ifndef UL_PANE_H
#define UL_PANE_H
#include "global.h"

namespace ady {
namespace ultimate {
    class ANYENGINE_EXPORT Pane{
    public:
        virtual QString groupname()=0;
        virtual QString name()=0;

    };
}
}
#endif // UL_PANE_H
