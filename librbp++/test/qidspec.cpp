#include <iostream>
#include <cassert>
#include "qidspec.h"

using namespace rbp;

int main(int argc, char ** argv) {
    int i;

    for (i = 1; i < argc; i++) {
        try {
            QidSpec spec(argv[i]);
            for (QidSpec::Iterator it = spec.begin(); it != spec.end(); it++) {
                std::cout << *it << " ";
                assert(spec.contains(*it));
            }
            std::cout << std::endl;
        } catch (QidSpecException qe) {
            std::cerr << "Parse exception on spec '" << argv[i] << "': "
                << qe.what() << std::endl;
        }
    }
}
