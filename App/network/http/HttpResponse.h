#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H
#include "global.h"
#include "network/NetworkResponse.h"
namespace ady{
    class ANYENGINE_EXPORT HttpResponse : public NetworkResponse
    {
    public:
        HttpResponse();

        virtual QList<FileItem> parseList();
        virtual void parse();
        virtual bool status();
    };


}
#endif // HTTPRESPONSE_H
