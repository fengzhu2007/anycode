#ifndef COSLISTOBJECTRESPONSE_H
#define COSLISTOBJECTRESPONSE_H
#include "cos_response.h"
namespace ady {
    class COSListObjectResponse : public COSResponse{
    public:
        COSListObjectResponse();
        virtual QList<FileItem> parseList() override;
        virtual bool status() override;
        inline void setStatus(bool status){m_status=status;}
        void setLists(QList<FileItem> lists);
        void appendLists(QList<FileItem> lists);
    private:
        bool m_status=false;
        QList<FileItem> m_lists;
    };
}

#endif // COSLISTOBJECTRESPONSE_H
