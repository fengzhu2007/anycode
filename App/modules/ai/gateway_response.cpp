#include "gateway_response.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace ady{

GatewayResponse::GatewayResponse() : HttpResponse() {

}

void GatewayResponse::parse(){
    HttpResponse::parse();
}

QString GatewayResponse::suggestion(){
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(this->body.toUtf8(), &error);
    if(error.error != QJsonParseError::NoError || !doc.isObject())
        return {};

    QJsonObject data = doc.object();
    QJsonArray choices = data.value("choices").toArray();
    if(choices.isEmpty())
        return {};

    QJsonObject message = choices.at(0).toObject().value("message").toObject();
    return message.value("content").toString();
}

}
