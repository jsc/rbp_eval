#include "tau_ap.h"

#include <iostream>

using namespace rbp;

int main(int argc, char ** argv) {
    CtBNode * n;

    n = new CtBNode(NULL, 1);
    n->add_descendant(2);
    n->add_descendant(5);
    n->add_descendant(6);
    std::cout << n->get_st_count() << std::endl;
    delete(n);
    return 0;
}
