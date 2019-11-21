#include "rungroups.h"

#include <sstream>

using namespace rbp;

void RunGroups::parse(std::istream &in) {
    std::string line;
    unsigned int linenum = 0;
    while (getline(in, line)) {
        if (line[0] == '#')
            continue;
        std::string::size_type colon = line.find(':');
        if (colon == std::string::npos) {
            std::ostringstream s;
            s << "Invalid format line " << linenum << ": '"
                << line << "'";
            throw RunGroupException(s.str().c_str());
        }
        std::string group_name = line.substr(0, colon);
        std::istringstream runids(line.substr(colon + 1));
        RunGroup group(group_name);
        std::string runid;
        while (runids >> runid) {
            group.add_runid(runid);
        }
        add_run_group(group);
    }
}
