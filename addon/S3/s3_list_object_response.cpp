#include "s3_list_object_response.h"
namespace ady{


    S3ListObjectResponse::S3ListObjectResponse()
        :S3Response(0){

    }

    QList<FileItem> S3ListObjectResponse::parseList(){
        return m_lists;
    }

    bool S3ListObjectResponse::status(){
        return m_status;
    }

    void S3ListObjectResponse::setLists(QList<FileItem> lists)
    {
        m_lists = lists;
    }

    void S3ListObjectResponse::appendLists(QList<FileItem> lists)
    {
        m_lists.append(lists);
    }


}
