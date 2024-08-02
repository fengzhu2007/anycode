#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H
#include "global.h"
#include "network/network_response.h"
namespace ady{
    class ANYENGINE_EXPORT HttpResponse : public NetworkResponse
    {
    public:
        explicit HttpResponse(long long id=0);

        virtual QList<FileItem> parseList();
        virtual void parse();
        virtual bool status();
    };


}
#endif // HTTP_RESPONSE_H
