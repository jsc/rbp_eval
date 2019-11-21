#include <iostream>
#include "rungroups.h"

using namespace rbp;

int main(int argc, char ** argv) {
    RunGroups rungroups;
    rungroups.parse(std::cin);
    RunGroups::RunGroupMap groups = rungroups.get_groups();
    for (RunGroups::RunGroupMap::iterator it = groups.begin();
      it != groups.end(); it++) {
        const std::string & name = it->first;
        const RunGroup & group = it->second;
        std::cout << name << ":";
        const RunGroup::RunidList & runids = group.get_runids();
        for (RunGroup::RunidList::iterator it = runids.begin();
          it != runids.end(); it++) {
            std::cout << " " << *it;
        }
        std::cout << std::endl;
    }
    return 0;
}
