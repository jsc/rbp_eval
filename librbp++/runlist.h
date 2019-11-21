#ifndef RBPCC_RUNLIST_H
#define RBPCC_RUNLIST_H

/*
 *  List of runs.
 *
 *  The runs themselves are from the C rbp library.
 */

#include <vector>
#include <stdexcept>

extern "C" {
#include <librbp/run.h>
}

namespace rbp {

class RunList {
    public:
        class ParseException : public std::invalid_argument {
            public:
                ParseException(const char * message) 
                    : invalid_argument(message) {}
        };
        typedef std::vector<run_t *> ListType;
        typedef ListType::iterator Iterator;

    private:
        ListType runs;

    public:
        // XXX destructor that frees the runs
        void add(run_t * run) {
            runs.push_back(run);
        }
        void load_runs(char ** runfiles, unsigned int num_runfiles);
        Iterator begin() { return runs.begin(); }
        Iterator end()   { return runs.end(); }
        unsigned num_runs() { return runs.size(); }
};
};


#endif /* RBPCC_RUNLIST_H */
