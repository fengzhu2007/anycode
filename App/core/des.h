#ifndef DES_H
#define DES_H
#include <QString>
#include "global.h"
namespace ady {
    class ANYENGINE_EXPORT DES {
    public:
        static QString encode(const QString& str,const QString &key);
        static QString decode(const QString& str,const QString &key);
        static QString encode(const QString& str);
        static QString decode(const QString& str);

    private:
        static QString vt;
        static QString key;
    };

};


#endif // DES_H
