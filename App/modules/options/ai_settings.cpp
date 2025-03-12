#include "ai_settings.h"




static const char enableKey[] = "Enable";
static const char tiggerPolicyKey[] = "TiggerPolicy";
static const char triggerTimeoutKey[] = "TriggerTimeoutKey";
static const char nameKey[] = "Name";
static const char modelKey[] = "Model";
static const char apiKeyKey[] = "APIKey";

namespace ady{


/*
    bool m_enable;
    int m_triggerPolicy;
    int m_triggerTimeout;//
    QString m_name;//
    QString m_model;
    QString m_apiKey;
*/

AISettings::AISettings():m_enable(false),m_triggerPolicy(Auto),m_triggerTimeout(1000) {


}



QJsonObject AISettings::toJson(){
    return {
        {enableKey,m_enable},
        {tiggerPolicyKey,m_triggerPolicy},
        {triggerTimeoutKey,m_triggerTimeout},
        {nameKey,m_name},
        {modelKey,m_model},
        {apiKeyKey,m_apiKey},
    };
}

void AISettings::fromJson(const QJsonObject& data){
    if(data.contains(enableKey)){
        m_enable = data.find(enableKey)->toBool(false);
    }
    if(data.contains(tiggerPolicyKey)){
        m_triggerPolicy = data.find(tiggerPolicyKey)->toInt(Auto);
    }
    if(data.contains(triggerTimeoutKey)){
        m_triggerTimeout = data.find(triggerTimeoutKey)->toInt(1000);
    }
    if(data.contains(nameKey)){
        m_name = data.find(nameKey)->toString();
    }
    if(data.contains(modelKey)){
        m_model = data.find(modelKey)->toString();
    }
    if(data.contains(apiKeyKey)){
        m_apiKey = data.find(apiKeyKey)->toString();
    }
}

QVariantMap AISettings::toMap() const{
    return {
        {enableKey,m_enable},
        {tiggerPolicyKey,m_triggerPolicy},
        {triggerTimeoutKey,m_triggerTimeout},
        {nameKey,m_name},
        {modelKey,m_model},
        {apiKeyKey,m_apiKey}
    };
}

void AISettings::fromMap(const QVariantMap &data){
    if(data.contains(enableKey)){
        m_enable = data.find(enableKey)->toBool();
    }
    if(data.contains(tiggerPolicyKey)){
        m_triggerPolicy = data.find(tiggerPolicyKey)->toInt();
    }
    if(data.contains(triggerTimeoutKey)){
        m_triggerTimeout = data.find(triggerTimeoutKey)->toInt();
    }
    if(data.contains(nameKey)){
        m_name = data.find(nameKey)->toString();
    }
    if(data.contains(modelKey)){
        m_model = data.find(modelKey)->toString();
    }
    if(data.contains(apiKeyKey)){
        m_apiKey = data.find(apiKeyKey)->toString();
    }
}

bool AISettings::equals(const AISettings &ts) const{
    return m_enable == ts.m_enable
           && m_triggerPolicy == ts.m_triggerPolicy
           && m_triggerTimeout == ts.m_triggerTimeout
           && m_name == ts.m_name
           && m_model == ts.m_model
        && m_apiKey == ts.m_apiKey;
}

QString AISettings::name(){
    return QLatin1String("ai");
}

}
