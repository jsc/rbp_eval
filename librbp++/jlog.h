#ifndef RBPCC_JLOG_H
#define RBPCC_JLOG_H

#include <string>
#include <vector>
#include <iostream>
#include "judgment.h"

/*
 *  A judgment log; that is, a record of the order in which
 *  documents were chosen for judgment according to some
 *  judgment strategy.
 */

namespace rbp {

typedef std::vector<Judgment> jvec;

class JudgmentLog {
    private:
        jvec judgments_;

    public:
        JudgmentLog() { }
        void load(std::istream &in);
        jvec::iterator begin() { return judgments_.begin(); }
        jvec::iterator end() { return judgments_.end(); }
};

}; /* end namespace */


#endif /* RBPCC_JLOG_H */
