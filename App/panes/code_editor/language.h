#ifndef LANGUAGE_H
#define LANGUAGE_H
#include "global.h"
#include <QVector>
namespace ady{
class ANYENGINE_EXPORT Language
{
public:
    Language();
    virtual ~Language();
    virtual QString name()=0;
    virtual QVector<QString> keywords()=0;
    virtual QVector<QString> classes()=0;
    virtual QVector<QString> functions()=0;
    virtual QVector<QString> variants()=0;
    virtual QVector<QString> constants()=0;
    virtual QVector<QString> fields()=0;



};
}

#endif // LANGUAGE_H
