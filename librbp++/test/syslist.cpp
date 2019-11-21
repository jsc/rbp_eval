#include "run.h"
#include "syslist.h"
#include "sysrank_topic.h"

#include <fstream>

using namespace rbp;

int main(int argc, char ** argv) {
    SysList syslist;
    SysRank info;

    try {
        //std::cout << "count = " << argc-1 << "argv = " << argv+1 << '\n';
        syslist.load_runs(argv+1, argc-1);
        //info = info.fill(syslist);
        //info.print_info(info);
        //std::cout << "num_runs = " << syslist.num_runs() << '\n';
    } catch (SysList::ParseException e) {
        std::cerr << e.what() << std::endl;
        return 0;
    }

    return 0;
}
