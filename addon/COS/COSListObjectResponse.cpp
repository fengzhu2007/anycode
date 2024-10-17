#include "COSListObjectResponse.h"
namespace ady{


    COSListObjectResponse::COSListObjectResponse()
        :COSResponse(0){

    }

    QList<FileItem> COSListObjectResponse::parseList(){
        return m_lists;
    }

    bool COSListObjectResponse::status(){
        return m_status;
    }

    void COSListObjectResponse::setLists(QList<FileItem> lists)
    {
        m_lists = lists;
    }

    void COSListObjectResponse::appendLists(QList<FileItem> lists)
    {
        m_lists.append(lists);
    }


}
