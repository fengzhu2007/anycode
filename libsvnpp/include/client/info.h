#ifndef INFO_H
#define INFO_H
#include "svnpp_global.h"
#include "types.h"
#include <string>



namespace ac {
namespace svn {
    class InfoPrivate;
    class LIBSVNPP_EXPORT Info{
    public:
        Info();
        ~Info();
        std::string url();
        std::string path();
        filesize_t filesize();


        void setUrl(std::string url);
        void setRootUrl(std::string root_url);

        void setPath(std::string path);
        void setFilesize(filesize_t filesize);

        void setUUID(std::string uuid);
        void setRevision(revnum_t rev);


        std::string toString();


    private:
        InfoPrivate* pri;

        friend class SvnppClient;

    };
}
}



//callback
#endif // INFO_H
