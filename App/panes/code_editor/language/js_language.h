#ifndef JSLANGUAGE_H
#define JSLANGUAGE_H

#include "../language.h"

namespace ady{
class ANYENGINE_EXPORT JsLanguage : public Language
{
public:
    JsLanguage();
    virtual ~JsLanguage();
    virtual QString name() override;
    virtual QVector<QString> keywords() override;
    virtual QVector<QString> classes() override;
    virtual QVector<QString> functions() override;
    virtual QVector<QString> variants() override;
    virtual QVector<QString> constants() override;
    virtual QVector<QString> fields() override;
};
}



#endif // JSLANGUAGE_H
