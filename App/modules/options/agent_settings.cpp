#include "agent_settings.h"

static const char portKey[] = "Port";
static const char runModeKey[] = "RunMode";
static const char proxyEnabledKey[] = "ProxyEnabled";
static const char autoFileDiffKey[] = "AutoFileDiff";

namespace ady{

AgentSettings::AgentSettings():m_port(3710),m_runMode(Terminal),m_proxyEnabled(false),m_autoFileDiff(false) {

}

QJsonObject AgentSettings::toJson(){
    return {
        {portKey,m_port},
        {runModeKey,m_runMode},
        {proxyEnabledKey,m_proxyEnabled},
        {autoFileDiffKey,m_autoFileDiff},
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
    if(data.contains(autoFileDiffKey)){
        m_autoFileDiff = data.find(autoFileDiffKey)->toBool(false);
    }
}

QVariantMap AgentSettings::toMap() const{
    return {
        {portKey,m_port},
        {runModeKey,m_runMode},
        {proxyEnabledKey,m_proxyEnabled},
        {autoFileDiffKey,m_autoFileDiff},
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
    if(data.contains(autoFileDiffKey)){
        m_autoFileDiff = data.find(autoFileDiffKey)->toBool();
    }
}

bool AgentSettings::equals(const AgentSettings &ts) const{
    return m_port == ts.m_port
           && m_runMode == ts.m_runMode
           && m_proxyEnabled == ts.m_proxyEnabled
           && m_autoFileDiff == ts.m_autoFileDiff;
}

QString AgentSettings::name(){
    return QLatin1String("agent");
}

}
