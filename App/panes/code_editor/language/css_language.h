#ifndef CSSLANGUAGE_H
#define CSSLANGUAGE_H

#include "../language.h"

namespace ady{
class ANYENGINE_EXPORT CssLanguage : public Language
{
public:
    CssLanguage();
    virtual ~CssLanguage();
    virtual QString name() override;
    virtual QVector<QString> keywords() override;
    virtual QVector<QString> classes() override;
    virtual QVector<QString> functions() override;
    virtual QVector<QString> variants() override;
    virtual QVector<QString> constants() override;
    virtual QVector<QString> fields() override;
};
}
#endif // CSSLANGUAGE_H
