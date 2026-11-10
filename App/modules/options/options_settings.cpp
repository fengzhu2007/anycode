#include "options_settings.h"
#include "storage/common_storage.h"
#include "environment_settings.h"
#include "language_settings.h"
#include "ai_settings.h"
#include "agent_settings.h"
#include "network_settings.h"
#include <texteditorsettings.h>

static const char environmentKey[] = "environment";
static const char languageKey[] = "language";
static const char aiKey[] = "ai";
static const char agentKey[] = "agent";
static const char networkKey[] = "network";


namespace ady{

OptionsSettings* OptionsSettings::instance = nullptr;

class OptionsSettingsPrivate{
public:
    EnvironmentSettings environmentSettings;
    LanguageSettings languageSettings;
    AISettings aiSettings;
    AgentSettings agentSettings;
    NetworkSettings networkSettings;
    QJsonObject data;

    QString empty_string;
    QStringList empty_stringlist;
    QMap<QVariant,QVariant> empty_map;
};


OptionsSettings::OptionsSettings():QObject() {
    d = new OptionsSettingsPrivate;
    d->data = CommonStorage().options();
    //NOTE: use value() instead of find(): when the database has no "options" record
    //(first run or a new data file), data is empty and find() returns end() which
    //must never be dereferenced.
    const QJsonObject options = d->data.value(QLatin1String("options")).toObject();

    if(options.contains(environmentKey)){
        d->environmentSettings.fromJson(options.value(environmentKey).toObject());
    }
    if(options.contains(languageKey)){
        d->languageSettings.fromJson(options.value(languageKey).toObject());
    }
    if(options.contains(aiKey)){
        d->aiSettings.fromJson(options.value(aiKey).toObject());
    }
    if(options.contains(agentKey)){
        d->agentSettings.fromJson(options.value(agentKey).toObject());
    }
    if(options.contains(networkKey)){
        d->networkSettings.fromJson(options.value(networkKey).toObject());
    }

    {
        const QString nameKey = TextEditor::TextEditorSettings::name();
        auto texteditorSettings = TextEditor::TextEditorSettings::instance();
        if(texteditorSettings==nullptr){
            texteditorSettings = new TextEditor::TextEditorSettings;
        }
        if(options.contains(nameKey)){
            texteditorSettings->fromJson(options.value(nameKey).toObject());
        }
    }
}


OptionsSettings* OptionsSettings::getInstance(){
    if(instance==nullptr){
        instance = new OptionsSettings;
    }
    return instance;
}

void OptionsSettings::init(){
    if(instance==nullptr){
        instance = new OptionsSettings;
    }
}

void OptionsSettings::destory(){
    delete instance;
    instance = nullptr;
}

OptionsSettings::~OptionsSettings(){
    delete d;
}

QJsonObject& OptionsSettings::data(){
    return d->data;
}

bool OptionsSettings::toBool(Option option){

    return false;
}

int OptionsSettings::toInt(Option option){

    return 0;
}

QString& OptionsSettings::toString(Option option) const{
    return d->empty_string;
}

QStringList& OptionsSettings::toStringList(Option option) const {
    return d->empty_stringlist;
}

QMap<QVariant,QVariant> OptionsSettings::toMap(Option option) const{

    return d->empty_map;
}

EnvironmentSettings& OptionsSettings::environmentSettings(){
    return d->environmentSettings;
}

void OptionsSettings::setEnvironmentSettings(const EnvironmentSettings& setting){
    if(d->environmentSettings==setting){
        return ;
    }
    d->environmentSettings = setting;
}

LanguageSettings& OptionsSettings::languageSettings(){
    return d->languageSettings;
}

void OptionsSettings::setLanguageSettings(const LanguageSettings& setting){
    if(d->languageSettings==setting){
        return ;
    }
    d->languageSettings = setting;
    emit languageChanged(d->languageSettings);
}

AISettings& OptionsSettings::aiSettings(){
    return d->aiSettings;
}

void OptionsSettings::setAiSettings(const AISettings& setting){
    if(d->aiSettings==setting){
        return ;
    }
    d->aiSettings = setting;
}

AgentSettings& OptionsSettings::agentSettings(){
    return d->agentSettings;
}

void OptionsSettings::setAgentSettings(const AgentSettings& setting){
    if(d->agentSettings==setting){
        return ;
    }
    d->agentSettings = setting;
}

NetworkSettings& OptionsSettings::networkSettings(){
    return d->networkSettings;
}

void OptionsSettings::setNetworkSettings(const NetworkSettings& setting){
    if(d->networkSettings==setting){
        return ;
    }
    d->networkSettings = setting;
}


}
