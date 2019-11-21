#ifndef RUN_H
#define RUN_H

#include <stdio.h>
#include "config.h"
#include "qdocs.h"

#if defined(HAVE_OPENSSL_MD5_H) && defined(HAVE_LIBSSL)
#define RUN_MD5SUM
#endif /* HAVE_OPENSSL_MD5_H && HAVE_LIBSSL*/

/*
 *  Output of a query evaluation run.
 */
typedef struct run run_t;

run_t * load_run(FILE * fp, char * err_buf, unsigned err_buf_len);

run_t * load_run_single_query(FILE * fp, char * qid, char * err_buf, 
  unsigned err_buf_len);

unsigned run_num_qdocs(run_t * run);

qdocs_t * run_get_qdocs_by_index(run_t * run, unsigned index);

qdocs_t * run_get_qdocs_by_qid(run_t * run, char * qid);

const char * run_get_runid(run_t * run);

#ifdef RUN_MD5SUM
char * run_get_md5sum(run_t * run);
#endif

void run_delete(run_t ** run);

#endif /* RUN_H */
