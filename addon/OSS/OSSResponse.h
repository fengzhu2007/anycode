#ifndef OSSRESPONSE_H
#define OSSRESPONSE_H
#include "network/http/HttpResponse.h"
#include <QMap>
namespace ady {
    class OSSResponse : public HttpResponse
    {
    public:
        OSSResponse();
        virtual QList<FileItem> parseList() override;
        //virtual void parse() override;
        virtual bool status() override;
    };
}
#endif // OSSRESPONSE_H
