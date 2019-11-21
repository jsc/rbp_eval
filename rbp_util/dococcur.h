#ifndef DOCOCCUR_H
#define DOCOCCUR_H

#include "array.h"
#include "run.h"
#include "strid.h"

typedef struct {
    unsigned rund;
    unsigned rank;
    void * data;
} dococcur_item_t;

ARRAY_TYPE_DECL(dococcur_item_array_t, dococcur_item_t);

/* Record which docs occur in which runs, and where. */

typedef struct dococcur dococcur_t;

dococcur_t * new_dococcur(void);

void dococcur_delete(dococcur_t ** dcr_p);

/*
 *  Record the fact that a <docid, qidd> pair occurs in a run.
 *
 *  RUND is the descriptor for the run; QIDD is the
 *  integer descriptor for the qid; RANK is the rank
 *  at which the document occurs.
 */
void dococcur_add(dococcur_t * dcr, const char * docid,
  unsigned qidd, unsigned rund, unsigned rank, void * data);

/*
 *  Record the document occurences for a run.
 */
void dococcur_add_run(dococcur_t * dcr, run_t * run, unsigned rund,
  strid_t * qidid);

/*
 *  Get the occurences of a document.
 */
dococcur_item_array_t * dococcur_get(dococcur_t * dcr, const char * docid,
  unsigned qidd);

#endif /* DOCOCCUR_H */
