#include "qdocs.h"
#include "dococcur.h"
#include "strhash.h"
#include "dqhash.h"
#include "util.h"

struct dococcur {
    strhash_t * hash;
};

void _free_dococcur_item_arr(void * data);

dococcur_t * new_dococcur(void) {
    dococcur_t * dcr;
    dcr = util_malloc_or_die(sizeof(*dcr));
    dcr->hash = new_strhash();
    return dcr;
}

void dococcur_delete(dococcur_t ** dcr_p) {
    dococcur_t * dcr;
    dcr = *dcr_p;
    strhash_delete(&dcr->hash, _free_dococcur_item_arr);
    free(dcr);
    *dcr_p = NULL;
}

void dococcur_add(dococcur_t * dcr, const char * docid,
  unsigned qidd, unsigned rund, unsigned rank, void * userdata) {
    dococcur_item_array_t * item_array;
    strhash_data_t * data;
    int found;
    dococcur_item_t item;

    data = dqhash_update(dcr->hash, docid, qidd, &found);
    if (found) {
        item_array = data->v;
    } else {
        item_array = util_malloc_or_die(sizeof(*item_array));
        ARRAY_INIT(*item_array);
        data->v = item_array;
    }
    item.rund = rund;
    item.rank = rank;
    item.data = userdata;
    ARRAY_ADD(*item_array, item);
}

void dococcur_add_run(dococcur_t * dw, run_t * run, unsigned rund,
  strid_t * qidid) {
    /* XXX the next 10 or so lines are repeated every time we
     * want to "get every score from a run". */
    unsigned q;
    unsigned num_qdocs;

    num_qdocs = run_num_qdocs(run);
    for (q = 0; q < num_qdocs; q++) {
        qdocs_t * qd = run_get_qdocs_by_index(run, q);
        unsigned num_scores = qdocs_num_scores(qd);
        doc_score_t * ds = qdocs_get_scores(qd, QDOCS_DEFAULT_ORDERING);
        unsigned s;

        unsigned qidd = strid_lookup_id(qidid, qdocs_qid(qd));
        if (qidd == UINT_MAX) {
            continue;
        }
        for (s = 0; s < num_scores; s++) {
            dococcur_add(dw, ds[s].docid, qidd, rund, s, &ds[s]);
        }
    }
}

dococcur_item_array_t * dococcur_get(dococcur_t * dcr, const char * docid,
  unsigned qidd) {
    int found;
    strhash_data_t data;

    data = dqhash_get(dcr->hash, docid, qidd, &found);
    if (!found)
        return NULL;
    else
        return data.v;
}

void _free_dococcur_item_arr(void * data) {
    dococcur_item_array_t * item_arr = data;
    free(item_arr->elems);
    free(item_arr);
}

#ifdef DOCOCCUR_MAIN

#include <assert.h>

int main(void) {
    dococcur_t * dcr;
    dococcur_item_array_t * da;

    dcr = new_dococcur();
    dococcur_add(dcr, "d1", 4, 0, 2, NULL);
    dococcur_add(dcr, "d1", 4, 0, 1, NULL);
    dococcur_add(dcr, "d2", 2, 1, 4, NULL);
    dococcur_add(dcr, "d2", 2, 10, 8, NULL);
    dococcur_add(dcr, "d1", 2, 11, 3, NULL);

    da = dococcur_get(dcr, "d1", 4);
    assert(da->elem_count == 2);

    assert(da->elems[0].rund == 0);
    assert(da->elems[0].rank == 2);

    assert(da->elems[1].rund == 0);
    assert(da->elems[1].rank == 1);

    da = dococcur_get(dcr, "d1", 2);
    assert(da->elem_count == 1);

    assert(da->elems[0].rund == 11);
    assert(da->elems[0].rank == 3);

    da = dococcur_get(dcr, "d2", 2);
    assert(da->elem_count == 2);

    assert(da->elems[0].rund == 1);
    assert(da->elems[0].rank == 4);

    assert(da->elems[1].rund == 10);
    assert(da->elems[1].rank == 8);

    da = dococcur_get(dcr, "d3", 0);
    assert(da == NULL);

    da = dococcur_get(dcr, "d1", 0);
    assert(da == NULL);

    dococcur_delete(&dcr);
    return 0;
}

#endif /* DOCOCCUR_MAIN */
