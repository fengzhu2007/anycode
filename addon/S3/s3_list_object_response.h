#ifndef S3LISTOBJECTRESPONSE_H
#define S3LISTOBJECTRESPONSE_H
#include "s3_response.h"
namespace ady {
    class S3ListObjectResponse : public S3Response{
    public:
        S3ListObjectResponse();
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

#endif // S3LISTOBJECTRESPONSE_H
