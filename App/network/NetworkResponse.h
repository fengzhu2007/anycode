#ifndef NETWORKRESPONSE_H
#define NETWORKRESPONSE_H
#include "global.h"
#include <QString>
#include "local/FileItem.h"
#include <QList>
#include <QMap>
#include <QVariant>
namespace ady {
    class ANYENGINE_EXPORT NetworkResponse
    {
    public:
        NetworkResponse();
        virtual ~NetworkResponse();
        virtual QList<FileItem> parseList() = 0;
        virtual void parse()=0;
        virtual void debug();
        virtual bool status()=0;

        void setCommand(QString command);



    public:
        int errorCode;
        int networkErrorCode;
        QString errorMsg;
        QString networkErrorMsg;

        QString header;
        QString body;
        QString command;

        QList<QString> headers;
        QMap<QString,QVariant> params;

    };
}
#endif // NETWORKRESPONSE_H
