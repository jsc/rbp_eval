#ifndef TREC_FMT_H
#define TREC_FMT_H

#include "fmt.h"

/* Results formatting functions. */

/* Format results in a manner similar to trec_eval's "relational" format */
void trec_fmt(res_t * res, fmt_args_t * args, FILE * fp);

#endif /* TREC_FMT_H */
