#include "language_manager.h"
#include "language.h"
#include "language/php_language.h"
#include "language/html_language.h"
#include "language/js_language.h"
#include "language/css_language.h"
#include <QMap>
namespace ady{

LanguageManager* LanguageManager::instance = nullptr;


class LanguageManagerPrivate{
public:
    QMap<QString,Language*>languages;
};

LanguageManager::LanguageManager()
{
    d = new LanguageManagerPrivate;
}


void LanguageManager::init(){
    if(instance==nullptr){
        instance = new LanguageManager;
    }
}

LanguageManager* LanguageManager::getInstance(){
    return instance;
}

void LanguageManager::destory(){
    if(instance==nullptr){
        delete instance;
        instance = nullptr;
    }
}

LanguageManager::~LanguageManager(){
    qDeleteAll(d->languages);
    delete d;
}

Language* LanguageManager::get(const QString& name){
    if(d->languages.contains(name)){
        return d->languages[name];
    }else{
        Language* language = nullptr;
        if(name=="php"){
            language = new PhpLanguage();
        }else if(name=="html" || name=="htm" || name=="shtml"){
            language = new HtmlLanguage();
        }else if(name=="js"){
            language = new JsLanguage();
        }else if(name=="css"){
            language = new CssLanguage();
        }
        if(language!=nullptr){
            d->languages.insert(language->name(),language);
        }
        return language;
    }
}

}
