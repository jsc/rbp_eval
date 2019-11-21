#include "parselist.h"
#include <iostream>
#include <vector>

#define MAX_LIST_SIZE 1024

using namespace rbp;

int main(int argc, char ** argv) {
    int i;
    std::vector<int> ints(MAX_LIST_SIZE);

    for (i = 1; i < argc; i++) {
        int elems = parse_list(argv[i], ints.begin(), ints.end());
        for (int e = 0; e < elems && e < ints.size(); e++) {
            std::cout << ints[e] << " ";
        }
        std::cout << std::endl;
    }
    return 0;
}
