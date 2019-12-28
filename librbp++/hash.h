#ifndef RBPCC_HASH_H
#define RBPCC_HASH_H

//#include <ext/hash_map>
#include <unordered_map>
#include <functional>

/*
 *  Nasty hackery, but it is noticeably faster.
 */
//template <> struct hash<std::string> {
//    size_t operator() (const std::string & s) const {
//        return __gnu_cxx::__stl_hash_string(s.c_str());
//    }
//};
//};

namespace rbp 
{

template <typename T>
struct hash 
{
};

template <> struct hash<std::string>
  {
    size_t operator()(std::string const & s) const
    {
      return std::hash<std::string>()(s);
    }
  };
};

namespace rbp {
    template <class Key, class Value> 
        struct Hash {
            typedef std::unordered_map<Key, Value,
            rbp::hash<Key>, std::equal_to<Key> > Type;
        };
};

#endif /* RBPCC_HASH_H */
