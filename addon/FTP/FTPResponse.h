#ifndef FTPRESPONSE_H
#define FTPRESPONSE_H
#include "network/NetworkResponse.h"
#include <QMap>
namespace ady {
    class FTPResponse : public NetworkResponse
    {
    public:

        FTPResponse();
        virtual QList<FileItem> parseList() override;
        virtual void parse() override;
        virtual bool status() override;



    };
}
#endif // FTPRESPONSE_H
