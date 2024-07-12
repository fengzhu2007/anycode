#ifndef HTMLLANGUAGE_H
#define HTMLLANGUAGE_H

#include "../language.h"

namespace ady{
class ANYENGINE_EXPORT HtmlLanguage : public Language
{
public:
    HtmlLanguage();
    virtual ~HtmlLanguage();
    virtual QString name() override;
    virtual QVector<QString> keywords() override;
    virtual QVector<QString> classes() override;
    virtual QVector<QString> functions() override;
    virtual QVector<QString> variants() override;
    virtual QVector<QString> constants() override;
    virtual QVector<QString> fields() override;
};
}

#endif // HTMLLANGUAGE_H
