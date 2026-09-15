#include "agent_settings.h"

static const char portKey[] = "Port";
static const char runModeKey[] = "RunMode";
static const char proxyEnabledKey[] = "ProxyEnabled";

namespace ady{

AgentSettings::AgentSettings():m_port(3710),m_runMode(Terminal),m_proxyEnabled(false) {

}

QJsonObject AgentSettings::toJson(){
    return {
        {portKey,m_port},
        {runModeKey,m_runMode},
        {proxyEnabledKey,m_proxyEnabled},
    };
}

void AgentSettings::fromJson(const QJsonObject& data){
    if(data.contains(portKey)){
        m_port = data.find(portKey)->toInt(3710);
    }
    if(data.contains(runModeKey)){
        m_runMode = data.find(runModeKey)->toInt(Terminal);
    }
    if(data.contains(proxyEnabledKey)){
        m_proxyEnabled = data.find(proxyEnabledKey)->toBool(false);
    }
}

QVariantMap AgentSettings::toMap() const{
    return {
        {portKey,m_port},
        {runModeKey,m_runMode},
        {proxyEnabledKey,m_proxyEnabled},
    };
}

void AgentSettings::fromMap(const QVariantMap &data){
    if(data.contains(portKey)){
        m_port = data.find(portKey)->toInt();
    }
    if(data.contains(runModeKey)){
        m_runMode = data.find(runModeKey)->toInt();
    }
    if(data.contains(proxyEnabledKey)){
        m_proxyEnabled = data.find(proxyEnabledKey)->toBool();
    }
}

bool AgentSettings::equals(const AgentSettings &ts) const{
    return m_port == ts.m_port
           && m_runMode == ts.m_runMode
           && m_proxyEnabled == ts.m_proxyEnabled;
}

QString AgentSettings::name(){
    return QLatin1String("agent");
}

}
