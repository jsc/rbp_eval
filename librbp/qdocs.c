#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "qdocs.h"
#include "util.h"

#define SCORES_INIT_SZ 1024
#define SCORES_EXP_FACTOR 2.0

struct qdocs {
    char * qid;
    doc_score_t * scores;
    unsigned scores_num;
    unsigned scores_size;
    enum qdocs_ord_t ord;
};

static void _qdocs_reorder(qdocs_t * qd, enum qdocs_ord_t ord);
typedef int (*cmp_fn_t)(const void *, const void *);
static int _ds_cmp_occur(const void *, const void *);
static int _ds_cmp_rank(const void *, const void *);
static int _ds_cmp_score(const void *, const void *);

qdocs_t * new_qdocs(char * qid) {
    qdocs_t * qd;
    qd = util_malloc_or_die(sizeof(*qd));
    qd->qid = util_strdup_or_die(qid);
    qd->scores = NULL;
    qd->scores_num = 0;
    qd->scores_size = 0;
    qd->ord = QDOCS_ORD_OCCUR;
    return qd;
}

void qdocs_delete(qdocs_t ** qd_p) {
    qdocs_t * qd = *qd_p;
    unsigned i;
    for (i = 0; i < qd->scores_num; i++) {
        free(qd->scores[i].docid);
    }
    free(qd->qid);
    free(qd->scores);
    free(qd);
    *qd_p = NULL;
}

void qdocs_add_doc_score(qdocs_t * qd, char * docid, unsigned rank,
  double score) {
    doc_score_t * ds; 
    /* XXX could check for duplicate score, but this would involve
     * keeping a strhash per qdoc. */
    util_ensure_array_space((void **) &qd->scores, &qd->scores_size,
      qd->scores_num, sizeof(*qd->scores), SCORES_INIT_SZ,
      SCORES_EXP_FACTOR);
    ds = &qd->scores[qd->scores_num];
    ds->docid = util_strdup_or_die(docid);
    util_downcase_str(ds->docid);
    ds->score = score;
    ds->rank = rank;
    ds->occur = qd->scores_num;
    ds->flags = 0;
    qd->scores_num++;
}

char * qdocs_qid(qdocs_t * qd) {
    return qd->qid;
}

unsigned qdocs_num_scores(qdocs_t * qd) {
    return qd->scores_num;
}

doc_score_t * qdocs_get_scores(qdocs_t * qd, enum qdocs_ord_t ord) {
    if (ord != qd->ord) {
        _qdocs_reorder(qd, ord);
    }
    assert(qd->ord == ord);
    return qd->scores;
}

static void _qdocs_reorder(qdocs_t * qd, enum qdocs_ord_t ord) {
    cmp_fn_t cmp_fn = NULL;
    switch (ord) {
    case QDOCS_ORD_OCCUR:
        cmp_fn = _ds_cmp_occur;
        break;
    case QDOCS_ORD_RANK:
        cmp_fn = _ds_cmp_rank;
        break;
    case QDOCS_ORD_SCORE:
        cmp_fn = _ds_cmp_score;
        break;
    }
    qsort(qd->scores, qd->scores_num, sizeof(*qd->scores), cmp_fn);
    qd->ord = ord;
}

static int _ds_cmp_occur(const void * va, const void * vb) {
    const doc_score_t * a = va;
    const doc_score_t * b = vb;

    return a->occur - b->occur;
}

static int _ds_cmp_rank(const void * va, const void * vb) {
    const doc_score_t * a = va;
    const doc_score_t * b = vb;

    return a->rank - b->rank;
}

static int _ds_cmp_score(const void * va, const void * vb) {
    const doc_score_t * a = va;
    const doc_score_t * b = vb;

    /* NOTE: sort by _decreasing_ score */
    if (a->score > b->score) {
        return -1;
    } else if (b->score > a->score) {
        return 1;
    } else {
        /* following trec_eval */
        return strcmp(b->docid, a->docid);
    }
}

#ifdef QDOCS_MAIN

#include <stdio.h>
#include "run.h"

int main(int argc, char ** argv) {
    char * fname;
    FILE * fp;
    run_t * run;
    unsigned qd_num, i;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <run-file>\n", argv[0]);
        return -1;
    }
    fname = argv[1];
    fp = fopen(fname, "r");
    if (fp == NULL) {
        fprintf(stderr, "Unable to open file %s for reading\n", fname);
        return -1;
    }
    run = load_run(fp, NULL, 0);
    if (run == NULL) {
        fprintf(stderr, "Error loading run file\n");
        fclose(fp);
        return -1;
    }
    qd_num = run_num_qdocs(run);
    for (i = 0; i < qd_num; i++) {
        qdocs_t * qd;
        doc_score_t * ds;
        unsigned num_scores;
        unsigned s;

        qd = run_get_qdocs_by_index(run, i);
        num_scores = qdocs_num_scores(qd);

        ds = qdocs_get_scores(qd, QDOCS_ORD_OCCUR);
        for (s = 1; s < num_scores; s++) {
            assert(ds[s].occur > ds[s - 1].occur);
        }

        ds = qdocs_get_scores(qd, QDOCS_ORD_RANK);
        for (s = 1; s < num_scores; s++) {
            assert(ds[s].rank >= ds[s - 1].rank);
        }

        ds = qdocs_get_scores(qd, QDOCS_ORD_SCORE);
        for (s = 1; s < num_scores; s++) {
            assert(ds[s].score >= ds[s - 1].score);
        }
    }
    run_delete(&run);
    fclose(fp);
    return 0;
}

#endif /* QDOCS_MAIN */
