#ifndef AGENT_SETTINGS_H
#define AGENT_SETTINGS_H

#include <QJsonObject>

namespace ady{
class AgentSettings
{
public:
    enum RunMode{
        Terminal=0,    // 在内置终端中运行
        QProcessMode   // 在QProcess中运行
    };

    AgentSettings();

    QJsonObject toJson();
    void fromJson(const QJsonObject& data);

    QVariantMap toMap() const;
    void fromMap(const QVariantMap &map);

    bool equals(const AgentSettings &ts) const;

    friend bool operator==(const AgentSettings &t1, const AgentSettings &t2) { return t1.equals(t2); }
    friend bool operator!=(const AgentSettings &t1, const AgentSettings &t2) { return !t1.equals(t2); }

    static QString name();

public:
    int m_port;
    int m_runMode;          // Terminal or QProcessMode
    bool m_proxyEnabled;    // 是否启用代理（复用网络设置中的代理配置）
};
}

#endif // AGENT_SETTINGS_H
