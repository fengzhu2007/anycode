#ifndef {TPL}RESPONSE_H
#define {TPL}RESPONSE_H
#include "network/NetworkResponse.h"
#include <QMap>
namespace ady {
    class {TPL}Response : public NetworkResponse
    {
    public:

        {TPL}Response();
        virtual QList<FileItem> parseList() override;
        virtual void parse() override;
        virtual bool status() override;



    };
}
#endif // {TPL}RESPONSE_H
