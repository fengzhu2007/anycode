#ifndef SFTPRESPONSE_H
#define SFTPRESPONSE_H
#include "network/network_response.h"
#include <QMap>
namespace ady {
    class SFTPResponse : public NetworkResponse
    {
    public:

        explicit SFTPResponse(long long id=0);
        virtual QList<FileItem> parseList() override;
        virtual void parse() override;
        virtual bool status() override;



    };
}
#endif // SFTPRESPONSE_H
