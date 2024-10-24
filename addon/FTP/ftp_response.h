#ifndef FTP_RESPONSE_H
#define FTP_RESPONSE_H
#include "network/network_response.h"
#include <QMap>
namespace ady {
    class FTPResponse : public NetworkResponse
    {
    public:

        explicit FTPResponse(long long id=0);
        virtual QList<FileItem> parseList() override;
        virtual void parse() override;
        virtual bool status() override;



    };
}
#endif // FTP_RESPONSE_H
