#ifndef DIFF_SUMMARY_H
#define DIFF_SUMMARY_H
#include "svnpp_global.h"
#include "types.h"
#include <string>
#include <vector>


namespace ac {
namespace svn{

    class DiffSummaryPrivate;
    class LIBSVNPP_EXPORT DiffSummary{
    public:
        DiffSummary();
        ~DiffSummary();
        int status();
        std::string path();

        void setStatus(int status);
        void setPath(std::string path);

    private:
        DiffSummaryPrivate* pri;

    };

    class DiffSummaryBaton{
    public:
        std::vector<DiffSummary*>lists;
        std::string anchor;
        bool igore_prototoies;
    };
}
}

#endif // DIFF_SUMMARY_H
