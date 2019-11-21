#ifndef OPT_H
#define OPT_H

#define DEFAULT_DEPTH   "0"
#define DEFAULT_PERSIST "0.5,0.8,0.95"

#include "depth.h"
#include "persist.h"
#include "fmt.h"
#include "qdocs.h"
#include "qrels.h"

/* runtime options */

struct opt {
    depth_t depth;
    persist_t persist;
    enum qdocs_ord_t ord;
    enum details_t details;
    enum reltype_t reltype;
    double reltype_arg;
    int no_header;
    int help_and_exit;
    int no_warnings;

    const char * qrels_fname;
    const char * run_fname;
};

void opt_init(struct opt * opt); 

void opt_set_defaults(struct opt * opt);

int opt_getopt(struct opt * opt, int argc, char * const argv[]);

int opt_process(struct opt * opt, int argc, char * const argv[]);

#endif /* OPT_H */
