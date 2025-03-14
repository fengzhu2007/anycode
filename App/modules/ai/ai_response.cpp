#include "ai_response.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace ady{
AIResponse::AIResponse():HttpResponse() {


}

void AIResponse::parse(){
    HttpResponse::parse();
}

QString AIResponse::suggestion(){
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(this->body.toUtf8(),&error);
    if(error.error==QJsonParseError::NoError){
        QJsonObject data = doc.object();
        auto choices = data.find("choices")->toArray();
        if(choices.size()>0){
            auto code = choices.at(0).toObject().find("text")->toString();
            return code;
        }
    }
    return {};
}

}
