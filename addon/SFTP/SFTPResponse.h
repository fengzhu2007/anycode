#ifndef SFTPRESPONSE_H
#define SFTPRESPONSE_H
#include "network/NetworkResponse.h"
#include <QMap>
namespace ady {
    class SFTPResponse : public NetworkResponse
    {
    public:

        SFTPResponse();
        virtual QList<FileItem> parseList() override;
        virtual void parse() override;
        virtual bool status() override;



    };
}
#endif // SFTPRESPONSE_H
