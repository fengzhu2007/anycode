#ifndef OSS_RESPONSE_H
#define OSS_RESPONSE_H
#include "network/http/http_response.h"
#include <QMap>
namespace ady {
    class OSSResponse : public HttpResponse
    {
    public:
        explicit OSSResponse(long long id=0);
        virtual QList<FileItem> parseList() override;
        //virtual void parse() override;
        virtual bool status() override;
    };
}
#endif // OSS_RESPONSE_H
