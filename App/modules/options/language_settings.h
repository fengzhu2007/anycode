#ifndef LANGUAGE_SETTINGS_H
#define LANGUAGE_SETTINGS_H
#include <QJsonObject>
namespace ady{
class LanguageSettings
{
public:
    LanguageSettings();


    QJsonObject toJson();
    void fromJson(const QJsonObject& data);

    QVariantMap toMap() const;
    void fromMap(const QVariantMap &map);

    bool equals(const LanguageSettings &ts) const;

    friend bool operator==(const LanguageSettings &t1, const LanguageSettings &t2) { return t1.equals(t2); }
    friend bool operator!=(const LanguageSettings &t1, const LanguageSettings &t2) { return !t1.equals(t2); }

    static QString name();



public:
    bool m_phpCmdChecking;
    QString m_phpCmdPath;
};
}
#endif // LANGUAGE_SETTINGS_H
