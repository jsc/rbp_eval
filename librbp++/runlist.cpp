#include <sstream>
#include "runlist.h"

#define ERROR_BUF_SIZE 1024

using namespace rbp;

void RunList::load_runs(char ** runfiles, unsigned int num_runfiles) {
    for (unsigned int r = 0; r < num_runfiles; r++) {
        run_t * run;
        FILE * fp;
        char error_buf[ERROR_BUF_SIZE];

        fp = fopen(runfiles[r], "r");
        if (fp == NULL) {
            std::ostringstream s;
            s << "Unable to open run file '" << runfiles[r] << "' for opening";
            throw RunList::ParseException(s.str().c_str());
        }
        run = load_run(fp, error_buf, ERROR_BUF_SIZE);
        fclose(fp);
        if (run == NULL) {
            std::ostringstream s;
            s << "Error loading run file '" << runfiles[r] << "': " <<
                error_buf;
            throw RunList::ParseException(s.str().c_str());
        }
        this->add(run);
    }
}
