#ifndef S3RESPONSE_H
#define S3RESPONSE_H
#include "network/http/http_response.h"
#include <QMap>
namespace ady {
    class S3Response : public HttpResponse
    {
    public:

        explicit S3Response(long long id=0);
        virtual QList<FileItem> parseList() override;
        virtual bool status() override;
        inline QString continuationToken(){return m_continuation_token;}

    private:
        QString m_continuation_token;
    };
}
#endif // S3RESPONSE_H
