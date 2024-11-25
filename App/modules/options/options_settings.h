#ifndef OPTIONS_SETTINGS_H
#define OPTIONS_SETTINGS_H

#include <QJsonObject>
namespace ady{
class EnvironmentSettings;
class OptionsSettingsPrivate;
class OptionsSettings
{
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
private:
    OptionsSettings();

private:
    static OptionsSettings* instance;
    OptionsSettingsPrivate* d;
};
}

#endif // OPTIONS_SETTINGS_H
