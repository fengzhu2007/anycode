#ifndef OPTIONS_SETTINGS_H
#define OPTIONS_SETTINGS_H

#include <QJsonObject>
namespace ady{
class EnvironmentSettings;
class LanguageSettings;
class AISettings;
class OptionsSettingsPrivate;
class OptionsSettings : public QObject
{
    Q_OBJECT
public:
    enum Option{
        Theme=0,
        Language,
        TexteditorFileLimit,
        AutoSave,
        AutoSaveInterval,
    };

    static OptionsSettings* getInstance();
    static void init();
    static void destory();
    ~OptionsSettings();
    QJsonObject& data();

    bool toBool(Option option);
    int toInt(Option option);
    QString& toString(Option option) const;
    QStringList& toStringList(Option option) const ;
    QMap<QVariant,QVariant> toMap(Option option) const;

    EnvironmentSettings& environmentSettings();
    void setEnvironmentSettings(const EnvironmentSettings& setting);
    LanguageSettings& languageSettings();
    void setLanguageSettings(const LanguageSettings& setting);

    AISettings& aiSettings();
    void setAiSettings(const AISettings& setting);

signals:
    void languageChanged(const LanguageSettings&);

private:
    OptionsSettings();

private:
    static OptionsSettings* instance;
    OptionsSettingsPrivate* d;
};
}

#endif // OPTIONS_SETTINGS_H
