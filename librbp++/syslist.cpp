#include "syslist.h"

#include <sstream>
#include <fstream>
#include <iostream>

#define ERROR_BUF_SIZE 1024

#define pass_copy
//#define pass_pointer

using namespace rbp;

void SysList::load_runs(char ** runfiles, unsigned int num_runfiles) {
    for (unsigned int r = 0; r < num_runfiles; r++) {
        System system;
        char error_buf[ERROR_BUF_SIZE];

        std::ifstream infile;
        infile.open(runfiles[r]);
        if (!infile.is_open()) {
            std::ostringstream s;
            s << "Unable to open run file '" << runfiles[r] << "' for reading";
            throw SysList::ParseException(s.str().c_str());
        }
        
        System::parse_run_file(system, infile);
        infile.close();
        if (system.get_error()) {
            std::ostringstream s;
            s << "Error loading run file '" << runfiles[r] << "': \n";
            throw SysList::ParseException(s.str().c_str());
        }

        #ifdef pass_copy
        this->add(system);
        #endif /* pass_copy */

        #ifdef pass_pointer
        /* XXX this must be incorrect... */
        this->add(&system);
        #endif /* pass_pointer */
    }
}
