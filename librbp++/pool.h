#ifndef RBPCC_POOL_H
#define RBPCC_POOL_H

#include <string>
#include <set>
#include <vector>
#include <map>

extern "C" {
#include "librbp/run.h"
#include "librbp/qdocs.h"
}

namespace rbp {

/* 
 *  A judgment pool, for an individual query.
 */
class Pool {
    public:
        typedef std::set<std::string> SetType;
        typedef SetType::iterator Iterator;
        
    private:
        std::string qid_;
        SetType docs_;

    public:
        Pool() { }
        void add(qdocs_t * qdocs, int depth=-1);
        void add(std::string docid) { docs_.insert(docid); }

        void get_docs(std::vector<std::string> &vec);
        Iterator begin() { return docs_.begin(); }
        Iterator end() { return docs_.end(); }
        unsigned size() const { return docs_.size(); }
};

/*
 *  Judgment pools for a set of queries.
 */
class PoolSet {
    public:
        typedef std::map<std::string, Pool> MapType;
        typedef MapType::iterator Iterator;

    private:
        MapType map_;

    public:
        void add(run_t * run, int depth=-1);
        Iterator begin() { return map_.begin(); }
        Iterator end() { return map_.end(); }
        Pool & operator[](std::string &qid) {
            return map_[qid];
        }
};

}; /* end namespace */


#endif /* POOL_H */
