#include "qrels.h"

#include <iostream>

using namespace rbp;

int main(int argc, char ** argv) {
    Qrelset qrelset;

    Qrelset::parse_qrels_file(qrelset, std::cin);
    return 0;
}
