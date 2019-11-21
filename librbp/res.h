#ifndef RES_H
#define RES_H

#include "depth.h"
#include "persist.h"
#include "qrels.h"
#include "run.h"

/* Structures to hold rbp evaluation results. */

typedef struct {
    double sum;
    double err;
} persist_res_t;

typedef struct {
    /* fractional if relevances are fractional. */
    double num_rel_ret;
    persist_res_t * persist_res;
} depth_res_t;

typedef struct {
    const char * qid;
    unsigned num_ret; 
    double num_rel; /* < 0.0 means "no judgments" */
    depth_res_t * depth_res;
} qid_res_t;

typedef struct {
    unsigned num_qid;
    depth_t * depth;
    persist_t * persist;
    qid_res_t * qid_res;
    qid_res_t ave_res;
} res_t;

/* Perform a full evaluation and retrieve the results.  These results
 * must be free'd using res_delete, and you must delete the results
 * before deleting or otherwise modifying the passed-in qrels, run,
 * persist, and depth objects. */
res_t * evaluate_res(qrels_t * qrels, run_t * run, enum qdocs_ord_t ord,
  persist_t * persist, depth_t * depth);

void res_delete(res_t ** res_p);

#endif /* RES_H */
