#ifndef LOG_ENTRY_H
#define LOG_ENTRY_H

#include "svnpp_global.h"
#include "types.h"
#include <string>


namespace ac {
namespace svn{
    class SvnppClient;
    class LogEntryPrivate;
    class LIBSVNPP_EXPORT LogEntry{
    public:
        LogEntry();
        ~LogEntry();
        revnum_t revision();
        std::string author();
        std::string message();
        std::string date();


        void setRevision(revnum_t rev);
        void setAuthor(std::string author);
        void setMessage(std::string message);
        void setDate(std::string date);

    private:
        LogEntryPrivate* pri;

    };
}
}


#endif // LOG_ENTRY_H
