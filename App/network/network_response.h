#ifndef NETWORK_RESPONSE_H
#define NETWORK_RESPONSE_H
#include "global.h"
#include <QString>
#include "interface/file_item.h"
#include <QList>
#include <QMap>
#include <QVariant>
namespace ady {
    class ANYENGINE_EXPORT NetworkResponse
    {
    public:
        explicit NetworkResponse(long long id=0);
        virtual ~NetworkResponse();
        virtual QList<FileItem> parseList() = 0;
        virtual void parse()=0;
        virtual void debug();
        virtual bool status()=0;

        void setCommand(QString command);

        QString errorInfo();



    public:
        int errorCode;
        int networkErrorCode;
        long long id;
        QString errorMsg;
        QString networkErrorMsg;

        QString header;
        QString body;
        QString command;

        QList<QString> headers;
        QMap<QString,QVariant> params;

    };
}
#endif // NETWORK_RESPONSE_H
