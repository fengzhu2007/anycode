#include "network_response.h"
#include <QDebug>
namespace ady {
    NetworkResponse::NetworkResponse(long long id)
    {
        this->id = id;
        this->errorCode = 0; //curl error code 0 equal ok;
        this->networkErrorCode = 0;//network type error  like http 404  500
    }


    void NetworkResponse::debug()
    {
        qDebug()<<"command:"<<this->command;
        qDebug()<<"errorCode:"<<this->errorCode<<";msg:"<<this->errorMsg;
        qDebug()<<"networkErrorCode:"<<this->networkErrorCode<<";msg:"<<this->networkErrorMsg;
        qDebug()<<"header:"<<this->header;
        qDebug()<<"body:"<<this->body;
    }

    void NetworkResponse::setCommand(QString command)
    {
        this->command = command;
    }

    NetworkResponse::~NetworkResponse()
    {
        headers.clear();
        params.clear();
    }

}
