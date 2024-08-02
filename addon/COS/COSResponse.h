#ifndef COSRESPONSE_H
#define COSRESPONSE_H
#include "network/http/http_response.h"
#include <QMap>
namespace ady {
    class COSResponse : public HttpResponse
    {
    public:

        explicit COSResponse(long long id=0);
        virtual QList<FileItem> parseList() override;
        //virtual void parse() override;
        virtual bool status() override;

        inline QString marker(){return m_marker;}

    private:
        QString m_marker;

    };
}
#endif // COSRESPONSE_H
