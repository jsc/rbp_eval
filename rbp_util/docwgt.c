#include <assert.h>
#include <stdlib.h>
#include "util.h"
#include "rbp.h"
#include "docwgt.h"
#include "strhash.h"
#include "dqhash.h"
#include "qdocs.h"

#ifndef MIN
#define MIN(a,b)((a) > (b) ? (b) : (a))
#endif

struct docwgt {
    double persist;
    double * rbpwgts;
    unsigned wgts_depth;
    strhash_t * dochash;
};

static void _docwgt_add_doc(docwgt_t * dw, const char * docid, unsigned qidd, 
  unsigned rank);
static int _docwgt_elem_cmp(const void * a, const void * b);

docwgt_t * new_docwgt(double persist) {
    docwgt_t * dw;
    dw = util_malloc_or_die(sizeof(*dw));
    dw->rbpwgts = NULL;
    dw->wgts_depth = 0;
    dw->dochash = new_strhash();
    dw->persist = persist;
    return dw;
}

void docwgt_add_run(docwgt_t * dw, run_t * run, strid_t * qidid) {
    unsigned q;
    unsigned num_qdocs;

    num_qdocs = run_num_qdocs(run);
    for (q = 0; q < num_qdocs; q++) {
        qdocs_t * qd = run_get_qdocs_by_index(run, q);
        unsigned num_scores = qdocs_num_scores(qd);
        doc_score_t * ds = qdocs_get_scores(qd, QDOCS_DEFAULT_ORDERING);
        unsigned s;
        unsigned qidd;
        
        qidd = strid_lookup_id(qidid, qdocs_qid(qd));
        if (qidd == UINT_MAX)
            continue;
        for (s = 0; s < num_scores; s++) {
            _docwgt_add_doc(dw, ds[s].docid, qidd, s);
        }
    }
}

unsigned docwgt_num_entries(docwgt_t * dw) {
    return strhash_num_entries(dw->dochash);
}

unsigned docwgt_get_entries(docwgt_t * dw, docwgt_elem_t * elems,
  unsigned elems_size, int sort) {
    unsigned e = 0;
    strhash_iter_t * iter;
    const char * key;
    unsigned qidd;
    strhash_data_t data;
    iter = strhash_get_iter(dw->dochash);
    while ( e < elems_size && (key = dqhash_iter_next(iter, &qidd, &data)) 
      != NULL) {
        elems[e].docid = key;
        elems[e].qidd = qidd;
        elems[e].wgt = data.lf;
        elems[e].flags = 0;
        e++;
    }
    strhash_iter_delete(&iter);
    if (sort) {
        qsort(elems, e, sizeof(*elems), _docwgt_elem_cmp);
    }
    return e;
}

void docwgt_delete(docwgt_t ** dw_p) {
    docwgt_t * dw = *dw_p;
    strhash_delete(&dw->dochash, NULL);
    free(dw->rbpwgts);
    free(dw);
    *dw_p = NULL;
}

static void _docwgt_add_doc(docwgt_t * dw, const char * docid, unsigned qidd,
  unsigned rank) {
    int found;
    strhash_data_t * val = dqhash_update(dw->dochash, docid, qidd, &found);
    if (rank >= dw->wgts_depth) {
        if (dw->wgts_depth == 0)
            dw->wgts_depth = 1000;
        else
            dw->wgts_depth *= 2;
        dw->rbpwgts = util_realloc_or_die(dw->rbpwgts, 
          sizeof(*dw->rbpwgts) * dw->wgts_depth);
        rbp_weights(dw->rbpwgts, dw->persist, dw->wgts_depth);
    }
    if (found) {
        val->lf += dw->rbpwgts[rank];
    } else {
        val->lf = dw->rbpwgts[rank];
    }
}

static int _docwgt_elem_cmp(const void * a, const void * b) {
    docwgt_elem_t * ea = (docwgt_elem_t *) a;
    docwgt_elem_t * eb = (docwgt_elem_t *) b;
    /* sorting in descending, not ascending, order */
    if (ea->wgt > eb->wgt)
        return -1;
    else if (ea->wgt < eb->wgt)
        return 1;
    else
        return 0;
}

#ifdef DOCWGT_MAIN

#include <string.h>

#define NUM_ELEMS 3

int main(void) {
    docwgt_t * dw;
    strhash_data_t val;
    int found;
    docwgt_elem_t elems[NUM_ELEMS];
    double wgts[] = {(1 - 0.95) * (1.0 + 0.95), (1 - 0.95) * (1.0)};

    dw = new_docwgt(0.95);
    _docwgt_add_doc(dw, "d1", 3, 0);
    _docwgt_add_doc(dw, "d2", 3, 0);
    _docwgt_add_doc(dw, "d2", 4, 2);
    _docwgt_add_doc(dw, "d1", 3, 1);
    assert(docwgt_num_entries(dw) == 3);
    val = dqhash_get(dw->dochash, "d1", 3, &found);
    assert(found);
    assert(val.lf == wgts[0]);
    val = dqhash_get(dw->dochash, "d2", 3, &found);
    assert(found);
    assert(val.lf == wgts[1]);
    val = dqhash_get(dw->dochash, "d2", 4, &found);
    assert(found);

    assert(docwgt_get_entries(dw, elems, NUM_ELEMS, 1) == 3);
    assert(strcmp(elems[0].docid, "d1") == 0);
    assert(strcmp(elems[1].docid, "d2") == 0);
    assert(strcmp(elems[2].docid, "d2") == 0);
    assert(elems[0].qidd == 3);
    assert(elems[1].qidd == 3);
    assert(elems[2].qidd == 4);
    assert(elems[0].wgt == wgts[0]);
    assert(elems[1].wgt == wgts[1]);
    docwgt_delete(&dw);
    return 0;
}

#endif /* DOCWGT_MAIN */
