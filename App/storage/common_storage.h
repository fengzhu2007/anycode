#ifndef COMMONSTORAGE_H
#define COMMONSTORAGE_H
#include <QString>
#include "storage.h"
#include "global.h"
namespace ady {
    class ANYENGINE_EXPORT CommonStorage : public Storage{
    public:
        constexpr const static char  TABLE_NAME[] = "common";
        constexpr const static char  COL_NAME[]   = "name";
        constexpr const static char  COL_VALUE[]  = "value";
        constexpr const static char  COL_GROUP[]  = "group";



    };
}
#endif // COMMONSTORAGE_H
