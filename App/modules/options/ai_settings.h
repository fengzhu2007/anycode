#ifndef AI_SETTINGS_H
#define AI_SETTINGS_H

#include <QJsonObject>

namespace ady{
class AISettings
{
public:
    enum Policy{
        Auto=0,
        Manual
    };
    AISettings();

    QJsonObject toJson();
    void fromJson(const QJsonObject& data);

    QVariantMap toMap() const;
    void fromMap(const QVariantMap &map);

    bool equals(const AISettings &ts) const;

    friend bool operator==(const AISettings &t1, const AISettings &t2) { return t1.equals(t2); }
    friend bool operator!=(const AISettings &t1, const AISettings &t2) { return !t1.equals(t2); }

    static QString name();

    static QList<QPair<QString,QString>> servers();
    static QList<QPair<QString,QString>> models(const QString& server);

    inline bool enable(){
        return m_enable && m_apiKey.isEmpty()==false;
    }

public:
    bool m_enable;
    int m_triggerPolicy;
    int m_triggerTimeout;//
    QString m_name;//
    QString m_model;
    QString m_apiKey;

};
}
#endif // AI_SETTINGS_H
