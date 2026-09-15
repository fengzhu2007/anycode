#include "network_settings.h"

static const char hostKey[] = "Host";
static const char portKey[] = "Port";
static const char usernameKey[] = "Username";
static const char passwordKey[] = "Password";
static const char gatewayEnabledKey[] = "GatewayEnabled";

namespace ady{

NetworkSettings::NetworkSettings():m_port(0),m_gatewayEnabled(false) {

}

QJsonObject NetworkSettings::toJson(){
    return {
        {hostKey,m_host},
        {portKey,m_port},
        {usernameKey,m_username},
        {passwordKey,m_password},
        {gatewayEnabledKey,m_gatewayEnabled},
    };
}

void NetworkSettings::fromJson(const QJsonObject& data){
    if(data.contains(hostKey)){
        m_host = data.find(hostKey)->toString();
    }
    if(data.contains(portKey)){
        m_port = data.find(portKey)->toInt(0);
    }
    if(data.contains(usernameKey)){
        m_username = data.find(usernameKey)->toString();
    }
    if(data.contains(passwordKey)){
        m_password = data.find(passwordKey)->toString();
    }
    if(data.contains(gatewayEnabledKey)){
        m_gatewayEnabled = data.find(gatewayEnabledKey)->toBool(false);
    }
}

QVariantMap NetworkSettings::toMap() const{
    return {
        {hostKey,m_host},
        {portKey,m_port},
        {usernameKey,m_username},
        {passwordKey,m_password},
        {gatewayEnabledKey,m_gatewayEnabled},
    };
}

void NetworkSettings::fromMap(const QVariantMap &data){
    if(data.contains(hostKey)){
        m_host = data.find(hostKey)->toString();
    }
    if(data.contains(portKey)){
        m_port = data.find(portKey)->toInt();
    }
    if(data.contains(usernameKey)){
        m_username = data.find(usernameKey)->toString();
    }
    if(data.contains(passwordKey)){
        m_password = data.find(passwordKey)->toString();
    }
    if(data.contains(gatewayEnabledKey)){
        m_gatewayEnabled = data.find(gatewayEnabledKey)->toBool();
    }
}

bool NetworkSettings::equals(const NetworkSettings &ts) const{
    return m_host == ts.m_host
           && m_port == ts.m_port
           && m_username == ts.m_username
           && m_password == ts.m_password
           && m_gatewayEnabled == ts.m_gatewayEnabled;
}

QString NetworkSettings::name(){
    return QLatin1String("network");
}

}
