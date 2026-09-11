#ifndef NETWORK_SETTINGS_H
#define NETWORK_SETTINGS_H

#include <QJsonObject>

namespace ady{
class NetworkSettings
{
public:
    NetworkSettings();

    QJsonObject toJson();
    void fromJson(const QJsonObject& data);

    QVariantMap toMap() const;
    void fromMap(const QVariantMap &map);

    bool equals(const NetworkSettings &ts) const;

    friend bool operator==(const NetworkSettings &t1, const NetworkSettings &t2) { return t1.equals(t2); }
    friend bool operator!=(const NetworkSettings &t1, const NetworkSettings &t2) { return !t1.equals(t2); }

    static QString name();

public:
    QString m_host;
    int m_port;
    QString m_username;
    QString m_password;
    bool m_gatewayEnabled;
    bool m_opencodeCppEnabled;
};
}

#endif // NETWORK_SETTINGS_H
