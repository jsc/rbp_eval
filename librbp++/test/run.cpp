#include "run.h"

#include <fstream>

using namespace rbp;

int main(int argc, char ** argv) {
    System sys;

    System::parse_run_file(sys, std::cin);
    //sys.dump(std::cout);

    return 0;
}
