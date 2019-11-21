#include <iostream>
#include <fstream>
#include <cstdlib>
#include "jlog.h"

using namespace rbp;

int main(int argc, char ** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <logfile>" << std::endl;
        exit(1);
    }
    std::string logfile(argv[1]);
    std::ifstream in(logfile.c_str(), std::ios::in);
    if (in.fail()) {
        std::cerr << "Unable to open jlog file '" << logfile 
            << "' for reading" << std::endl;
    }
    JudgmentLog jlog;
    jlog.load(in);
    for (jvec::iterator it = jlog.begin(); it != jlog.end(); it++) {
        std::cout << it->get_qid() << " " << it->get_docid() << " "
            << it->get_rel() << std::endl;
    }
    return 0;
}
