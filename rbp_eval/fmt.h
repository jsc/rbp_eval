#ifndef FMT_H
#define FMT_H

/* opt.h needs to see this. */
enum details_t {
    DETAILS_AVERAGES = (1 << 0),
    DETAILS_PER_QUERY = (1 << 1)
};

#include <stdio.h>
#include "opt.h"
#include "res.h"
#include "run.h"

typedef struct {
    enum details_t details;
    int argc;
    char ** argv;
    struct opt * opt;
    run_t * run;
} fmt_args_t;

typedef void (*fmt_fn)(res_t * res, fmt_args_t * args, FILE * fp);

#endif /* FMT_H */
