#ifndef LANGUAGEMANAGER_H
#define LANGUAGEMANAGER_H
#include "global.h"
namespace ady{
class Language;
class LanguageManagerPrivate;
class ANYENGINE_EXPORT LanguageManager
{
public:
    static void init();
    static LanguageManager* getInstance();
    static void destory();
    ~LanguageManager();
    Language* get(const QString& name);

private:
    LanguageManager();


private:
    static LanguageManager* instance;
    LanguageManagerPrivate* d;
};
}
#endif // LANGUAGEMANAGER_H
