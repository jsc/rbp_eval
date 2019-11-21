#ifndef RBPCC_HASH_H
#define RBPCC_HASH_H

#include <ext/hash_map>

namespace __gnu_cxx {

/*
 *  Nasty hackery, but it is noticeably faster.
 */
template <> struct hash<std::string> {
    size_t operator() (const std::string & s) const {
        return __gnu_cxx::__stl_hash_string(s.c_str());
    }
};
};

namespace rbp {
    template <class Key, class Value> 
        struct Hash {
            typedef __gnu_cxx::hash_map<Key, Value,
             __gnu_cxx::hash<Key>, __gnu_cxx::equal_to<Key> > Type;
        };
};

#endif /* RBPCC_HASH_H */
