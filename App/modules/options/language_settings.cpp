#include "language_settings.h"


static const char phpCmdCheckingKey[] = "PHPCmdChecking";
static const char phpCmdPathKey[] = "PHPCmdPath";


namespace ady{

LanguageSettings::LanguageSettings():m_phpCmdChecking(false) {


}

QJsonObject LanguageSettings::toJson(){
    return {
        {phpCmdCheckingKey,m_phpCmdChecking},
        {phpCmdPathKey,m_phpCmdPath}
    };
}

void LanguageSettings::fromJson(const QJsonObject& data){
    if(data.contains(phpCmdCheckingKey)){
        m_phpCmdChecking = data.find(phpCmdCheckingKey)->toBool(false);
    }
    if(data.contains(phpCmdPathKey)){
        m_phpCmdPath = data.find(phpCmdPathKey)->toString();
    }

}

QVariantMap LanguageSettings::toMap() const{
    return {
        {phpCmdCheckingKey,m_phpCmdChecking},
        {phpCmdPathKey,m_phpCmdPath}
    };
}

void LanguageSettings::fromMap(const QVariantMap &data){
    if(data.contains(phpCmdCheckingKey)){
        m_phpCmdChecking = data.find(phpCmdCheckingKey)->toBool();
    }
    if(data.contains(phpCmdPathKey)){
        m_phpCmdPath = data.find(phpCmdPathKey)->toString();
    }
}

bool LanguageSettings::equals(const LanguageSettings &ts) const{
    return m_phpCmdChecking == ts.m_phpCmdChecking
           && m_phpCmdPath == ts.m_phpCmdPath;
}

QString LanguageSettings::name(){
    return QLatin1String("language");
}

}


