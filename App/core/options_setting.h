#ifndef OPTIONS_SETTING_H
#define OPTIONS_SETTING_H

#include <QJsonObject>
namespace ady{
class OptionsSettingPrivate;
class OptionsSetting
{
public:
    enum Option{
        Theme=0,
        Language,
        TexteditorFileLimit,
        AutoSave,
        AutoSaveInterval,
    };

    static OptionsSetting* getInstance();
    static void init();
    static void destory();
    ~OptionsSetting();
    QJsonObject& data();

    bool toBool(Option option);
    int toInt(Option option);
    QString& toString(Option option) const;
    QStringList& toStringList(Option option) const ;
    QMap<QVariant,QVariant> toMap(Option option) const;

private:
    OptionsSetting();

private:
    static OptionsSetting* instance;
    OptionsSettingPrivate* d;
};
}

#endif // OPTIONS_SETTING_H
