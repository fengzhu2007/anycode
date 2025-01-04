#include "environment_settings.h"

static const char themeKey[] = "Theme";
static const char languageKey[] = "Language";
static const char texteditorFileLimitKey[] = "TexteditorFileLimit";
static const char autoSaveKey[] = "AutoSave";
static const char autoSaveIntervalKey[] = "AutoSaveInterval";


namespace ady{



EnvironmentSettings::EnvironmentSettings():m_theme(QLatin1String("light")),m_language(QLatin1String("en")),m_texteditorFileLimit(2),m_autoSave(false),m_autoSaveInterval(5) {



}

QJsonObject EnvironmentSettings::toJson(){
    return {
        {themeKey,m_theme},
        {languageKey,m_language},
        {texteditorFileLimitKey,m_texteditorFileLimit},
        {autoSaveKey,m_autoSave},
        {autoSaveIntervalKey,m_autoSaveInterval}
    };
}

void EnvironmentSettings::fromJson(const QJsonObject& data){
    if(data.contains(themeKey)){
        m_theme = data.find(themeKey)->toString();
    }
    if(data.contains(languageKey)){
        m_language = data.find(languageKey)->toString();
    }
    if(data.contains(texteditorFileLimitKey)){
        m_texteditorFileLimit = data.find(texteditorFileLimitKey)->toInt();
    }
    if(data.contains(autoSaveKey)){
        m_autoSave = data.find(autoSaveKey)->toBool(false);
    }
    if(data.contains(autoSaveIntervalKey)){
        m_autoSaveInterval = data.find(autoSaveIntervalKey)->toInt();
    }
}

QVariantMap EnvironmentSettings::toMap() const{
    return {
        {themeKey,m_theme},
        {languageKey,m_language},
        {texteditorFileLimitKey,m_texteditorFileLimit},
        {autoSaveKey,m_autoSave},
        {autoSaveIntervalKey,m_autoSaveInterval}
    };
}

void EnvironmentSettings::fromMap(const QVariantMap &data){
    if(data.contains(themeKey)){
        m_theme = data.find(themeKey)->toString();
    }
    if(data.contains(languageKey)){
        m_language = data.find(languageKey)->toString();
    }
    if(data.contains(texteditorFileLimitKey)){
        m_texteditorFileLimit = data.find(texteditorFileLimitKey)->toInt();
    }
    if(data.contains(autoSaveKey)){
        m_autoSave = data.find(autoSaveKey)->toBool();
    }
    if(data.contains(autoSaveIntervalKey)){
        m_autoSaveInterval = data.find(autoSaveIntervalKey)->toInt();
    }
}

bool EnvironmentSettings::equals(const EnvironmentSettings &ts) const{
    return m_theme == ts.m_theme
           && m_language == ts.m_language
           && m_texteditorFileLimit == ts.m_texteditorFileLimit
           && m_autoSave == ts.m_autoSave
           && m_autoSaveInterval == ts.m_autoSaveInterval;
}

QString EnvironmentSettings::name(){
    return QLatin1String("environment");
}

QList<QPair<QString,QString>> EnvironmentSettings::themes(){
    return {
            {"default","Default Light"},
            {"dark","Dark"},
        };
}

QList<QPair<QString,QString>> EnvironmentSettings::languages(){
    return {
            {"en",QObject::tr("English")},
            {"zh_CN",QObject::tr("Chinese")},
        };
}

}
