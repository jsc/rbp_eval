#ifndef RBPCC_SYSLIST_H
#define RBPCC_SYSLIST_H

/*
 *  List of systems.
 */

#include "run.h"
#include <vector>
#include <stdexcept>

#define pass_copy
//#define pass_pointer

//extern "C" {
//#include <librbp/run.h>
//}

namespace rbp {

class SysList {
    public:
        class ParseException : public std::invalid_argument {
            public:
                ParseException(const char * message) 
                    : invalid_argument(message) {}
        };
        #ifdef pass_copy
        typedef std::vector<System> ListType;
        #endif /* pass_copy */

        #ifdef pass_pointer
        typedef std::vector<System *> ListType;
        #endif /* pass_pointer */

        typedef ListType::iterator Iterator;

    private:
        ListType systems;

    public:
        // XXX destructor that frees the systems
        void clear() { systems.clear(); }

        #ifdef pass_copy
        void add(System system) {
            systems.push_back(system);
        }
        #endif /* pass_copy */
        #ifdef pass_pointer
        void add(System *system) {
            systems.push_back(system);
        }
        #endif /* pass_pointer */

        void load_runs(char ** runfiles, unsigned int num_runfiles);
        Iterator begin() { return systems.begin(); }
        Iterator end()   { return systems.end(); }
        unsigned num_runs() { return systems.size(); }
};
};


#endif /* RBPCC_SYSLIST_H */

/*
 * Timings for <System *> :
 *  real    0m15.966s
 *  user    0m15.751s
 *  sys     0m0.214s
 *
 *  real    0m15.999s
 *  user    0m15.784s
 *  sys     0m0.217s
 *
 *  Timings for <System> :
 *  real    0m19.018s
 *  user    0m18.403s
 *  sys     0m0.613s
 *
 *  real    0m19.141s
 *  user    0m18.543s
 *  sys     0m0.599s
 */
