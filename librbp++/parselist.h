#ifndef RBPCC_PARSELIST_H
#define RBPCC_PARSELIST_H

#include <string>
#include <iostream>
#include <sstream>

/* Parse a comma-separated list of values into, well, a list of values */

namespace rbp {

template <class Out> unsigned parse_list(const char * str, Out start, 
  Out end) {
    std::string s(str);
    unsigned elements = 1;
    for (std::string::iterator i = s.begin(); i != s.end(); i++) {
        if (*i == ',') {
            *i = ' ';
            elements++;
        } 
    }
    std::istringstream is(s);
    for ( ; start < end; start++) {
        is >> *start;
    }
    return elements;
}

};

#endif /* RBPCC_PARSELIST_H */
