#ifndef FTPRESPONSE_H
#define FTPRESPONSE_H
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
#endif // FTPRESPONSE_H
