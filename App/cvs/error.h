#ifndef ERROR_H
#define ERROR_H
#include "global.h"
#include <QString>


namespace ady {
namespace cvs {
class ANYENGINE_EXPORT Error
{
public:
    Error();
    Error(int code,const QString& message);

public:
    int code;
    QString message;
};
}
}

#endif // ERROR_H
