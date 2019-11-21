#ifndef DOCWGT_H
#define DOCWGT_H

#include "run.h"
#include "strid.h"

typedef struct {
    const char * docid;
    unsigned qidd;
    double wgt;
    unsigned flags;
} docwgt_elem_t;

/* Take a list of runs, and determine which documents have the
 * highest "value" across the runs. */

typedef struct docwgt docwgt_t;

/* Create a new docwgt object. 
 * 
 * PERSIST is the user persistence (p) value.
 * DEPTH is the depth to which docwgts will be assessed.*/
docwgt_t * new_docwgt(double p);

/*
 *  Add the documents from a run to the docwgts.
 */
void docwgt_add_run(docwgt_t * dw, run_t * run, strid_t * qidid);

/*
 *  Get the total number of documents in the docwgt object.
 */
unsigned docwgt_num_entries(docwgt_t * dw);

/*
 *  Get the entries in the docmap.
 *
 *  SORTED indicates whether to sort the entries in descending
 *  wgt or not.
 */
unsigned docwgt_get_entries(docwgt_t * dw, docwgt_elem_t * elems,
  unsigned num_elems, int sort);

/*
 *  Delete a docwgt object.
 */
void docwgt_delete(docwgt_t ** dw_p);

#endif /* DOCWGT_H */
