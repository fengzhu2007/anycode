#ifndef LIBSVNPP_H
#define LIBSVNPP_H

#include "svnpp_global.h"
#include "types.h"
#include <string>
#include <vector>
namespace ac {
namespace svn {
class Info;
class LogEntry;
class DiffSummary;
class SvnppClientPrivate;
class LIBSVNPP_EXPORT SvnppClient
{
public:
    SvnppClient(std::string path);
    ~SvnppClient();

    bool online();
    void setOnline(bool online);



    //svn command methods
    int init(std::string path);
    Info* info();
    std::vector<LogEntry*> logs(revnum_t from,unsigned int limit);
    std::vector<DiffSummary*> diff(revnum_t start,revnum_t end);
    std::vector<DiffSummary*> status();
    std::pair<int,std::string> error() const;

private:
    SvnppClientPrivate* pri;

};


}
}

#endif // LIBSVNPP_H
