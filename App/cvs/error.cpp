#include "error.h"


namespace ady {
namespace cvs {

Error::Error():code(0){

}

Error::Error(int code,const QString& message)
    :code(code),message(message)
{

}

}}
