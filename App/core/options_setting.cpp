#include "options_setting.h"
#include "storage/common_storage.h"

namespace ady{

OptionsSetting* OptionsSetting::instance = nullptr;

class OptionsSettingData{
public:
    QStringList themes = {QObject::tr("Default")};
    QStringList languages = {QObject::tr("English"),QObject::tr("Chinese")};

};

class OptionsSettingPrivate{
public:
    int theme;
    int language;
    int texteditor_file_limit;//unit MB
    bool auto_save;
    int auto_save_interval;//unit minute


    QString empty_string;
    QStringList empty_stringlist;
    QMap<QVariant,QVariant> empty_map;

    QJsonObject data;
};


OptionsSetting::OptionsSetting() {
    d = new OptionsSettingPrivate;
    d->data = CommonStorage().options();
    if(d->data.isEmpty()){
        d->theme = 0;
        d->language = 0;
        d->texteditor_file_limit = 2;
        d->auto_save = false;
        d->auto_save_interval = 5;
    }else{
        d->theme = d->data.find("theme")->toInt(0);
        d->language = d->data.find("language")->toInt(0);
        d->texteditor_file_limit = d->data.find("texteditor_file_limit")->toInt(2);
        d->auto_save = d->data.find("auto_save")->toBool();
        d->auto_save_interval = d->data.find("auto_save_interval")->toInt(5);
    }
}


OptionsSetting* OptionsSetting::getInstance(){
    if(instance==nullptr){
        instance = new OptionsSetting;
    }
    return instance;
}

void OptionsSetting::init(){
    if(instance==nullptr){
        instance = new OptionsSetting;
    }
}

void OptionsSetting::destory(){
    delete instance;
    instance = nullptr;
}

OptionsSetting::~OptionsSetting(){
    delete d;
}

QJsonObject& OptionsSetting::data(){
    return d->data;
}

bool OptionsSetting::toBool(Option option){
    if(option==AutoSave){
        return d->auto_save;
    }
    return false;
}

int OptionsSetting::toInt(Option option){
    switch(option){
    case Theme:
        return d->theme;
    case Language:
        return d->language;
    case AutoSaveInterval:
        return d->auto_save_interval;
    case TexteditorFileLimit:
        return d->texteditor_file_limit;
    default:
        return 0;
    }
    return 0;
}

QString& OptionsSetting::toString(Option option) const{
    return d->empty_string;
}

QStringList& OptionsSetting::toStringList(Option option) const {
    return d->empty_stringlist;
}

QMap<QVariant,QVariant> OptionsSetting::toMap(Option option) const{

    return d->empty_map;
}


}
