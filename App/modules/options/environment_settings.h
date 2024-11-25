#ifndef ENVIRONMENT_SETTINGS_H
#define ENVIRONMENT_SETTINGS_H

#include <QJsonObject>

namespace ady{
class EnvironmentSettings
{
public:
    EnvironmentSettings();


    QJsonObject toJson();
    void fromJson(const QJsonObject& data);

    QVariantMap toMap() const;
    void fromMap(const QVariantMap &map);

    bool equals(const EnvironmentSettings &ts) const;

    friend bool operator==(const EnvironmentSettings &t1, const EnvironmentSettings &t2) { return t1.equals(t2); }
    friend bool operator!=(const EnvironmentSettings &t1, const EnvironmentSettings &t2) { return !t1.equals(t2); }

    static QString name();

    static QList<QPair<QString,QString>> themes();
    static QList<QPair<QString,QString>> languages();

public:
    QString m_theme;
    QString m_language;
    int m_texteditorFileLimit;//unit MB
    bool m_autoSave;
    int m_autoSaveInterval;//unit minute

};
}

#endif // ENVIRONMENT_SETTINGS_H
