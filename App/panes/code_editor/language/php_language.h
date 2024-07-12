#ifndef PHPLANGUAGE_H
#define PHPLANGUAGE_H

#include "../language.h"

namespace ady{
class ANYENGINE_EXPORT PhpLanguage : public Language
{
public:
    PhpLanguage();
    virtual ~PhpLanguage();
    virtual QString name() override;
    virtual QVector<QString> keywords() override;
    virtual QVector<QString> classes() override;
    virtual QVector<QString> functions() override;
    virtual QVector<QString> variants() override;
    virtual QVector<QString> constants() override;
    virtual QVector<QString> fields() override;
};
}

#endif // PHPLANGUAGE_H
