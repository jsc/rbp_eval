#include <math.h>
#include <assert.h>
#include <limits.h>
#include "rbp.h"
#include "util.h"

#ifndef MAX
#define MAX(a,b)((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)((a) > (b) ? (b) : (a))
#endif

struct rbp {
    qdocs_t * qdocs;
    enum qdocs_ord_t ord;
    qrels_t * qrels;
    persist_t * persist;
    unsigned depth;
    double num_rel_ret;
    rbp_val_t * vals;

    /* Handle ties */
    unsigned tie_len;
    union {
        unsigned rank;
        double score;
    } tie_val;
    unsigned tie_pos;
    double tie_accum_score;
};

rbp_t * new_rbp(qrels_t * qrels, enum qdocs_ord_t ord, persist_t * persist) {
    rbp_t * rbp = util_malloc_or_die(sizeof(*rbp));
    rbp->qdocs = NULL;
    rbp->qrels = qrels;
    rbp->ord = ord;
    rbp->persist = persist;
    rbp->depth = 0;
    rbp->num_rel_ret = 0.0;
    rbp->vals = util_malloc_or_die(sizeof(*rbp->vals) * persist->p_num);
    rbp->tie_len = 0;
    rbp->tie_pos = 0;
    switch (ord) {
    case QDOCS_ORD_OCCUR:
        break;
    case QDOCS_ORD_RANK:
        rbp->tie_val.rank = UINT_MAX;
        break;
    case QDOCS_ORD_SCORE:
        rbp->tie_val.score = -1.0;
        break;
    }
    return rbp;
}

int rbp_init(rbp_t * rbp, qdocs_t * qdocs) {
    int i;
    if (qrels_get_qid_qrels(rbp->qrels, qdocs_qid(qdocs)) == NULL) {
        return -1;
    }
    rbp->qdocs = qdocs;
    for (i = 0; i < rbp->persist->p_num; i++) {
        rbp->vals[i].sum = rbp->vals[i].err = rbp->vals[i].cumerr = 0.0;
        rbp->vals[i].wgt = 1 - rbp->persist->p[i];
    }
    rbp->depth = 0;
    rbp->num_rel_ret = 0.0;
    rbp->tie_len = 0;
    rbp->tie_pos = 0;
    return 0;
}

/* The following function is rather complex because Creepo, the
 * Muse of Program Specifications, has demanded: 
 * 
 *    1. that tied rankings be supported,
 *    2. that evaluation be cutoffable at arbitrary depths,
 *    3. that tied rankings that span the evaluation depth cutoff
 *          be handled, and
 *    4. that multiple evaluation depth cutoffs be supported, with
 *          calculation of each cutoff continuing from where the
 *          previous one left off.
 *
 * Tied rankings are handled by assigning to each element of the
 * tie the average weight for the positions over which the tie
 * extends.  Cutoffs in the middle of a tie are handled by
 * including all tied elements before the cutoff, but additionally
 * weighting each by the proportion of the tie that fits into
 * the cutoff.  Restartable multi-depth evaluations mean that
 * the span and weights of ties that are hanging over from the
 * previous depth cutoff have to be incorporated into the
 * following one.  In degenerative cases, it may even happen
 * that for a given continued evaluation, a tie may start before
 * the continued-from cutoff point, stretch across the span
 * of the continued evaluation, and extend beyond the continuation's
 * end cutoff point.
 *
 * Consider, for instance, an evaluation being made to depths 2, 5, and
 * 10, on a ranking where there are five document tied at rank 2.
 * Let w(n) be the weighting given to a document at rank n.
 * Every document at rank 2 would be given a base weighting of
 * w' = ( w(2) + w(3) + w(4) + w(5) + w(6) ) / 5.  For the evaluation
 * to depth 2, one-fifth of the tie at positions 2-6 fits in to
 * the evaluation's segment, so the five documents at rank 2 are
 * all assessed, each contributing a weighting of w' / 5.  Then
 * when the evaluation is continued to depth 5, three-fifths of
 * the tie fits into the extent of the next evaluation segment,
 * so the five documents at rank 2 are assessed again, each
 * contributing a weight of 3w' / 5.  Finally, the evaluation is
 * continued to depth 10, with one-fifth of the tie fitting into
 * the final segment, so the five documents at rank 2 are assessed
 * once more, each contributing a weight of w'/5.
 *
 * Note that at each depth cutoff at depth d, the residual uncertainty
 * always p^d, even if there is a tie that extends across the 
 * boundary at d.
 *
 * The effect of the above procedure in handling ties that occur
 * at evaluation cutoff boundaries is to produce the average rbp
 * values that would occur from randomly choosing tied elements to
 * make up the depth.  As a result, if such a tie occurs, the
 * result will not be the same as evaluating the head d documents
 * of the ranking.
 */

rbp_val_t * rbp_calc_to_depth(rbp_t * rbp, unsigned depth,
  double * num_rel_ret) {
    unsigned d, p, num_scores;
    char * qid;
    doc_score_t * doc_scores;
    qid_qrels_t * qq;
    unsigned actual_depth;

    assert(depth > rbp->depth);
    assert(rbp->qdocs != NULL);
    num_scores = qdocs_num_scores(rbp->qdocs);
    doc_scores = qdocs_get_scores(rbp->qdocs, rbp->ord);
    qid = qdocs_qid(rbp->qdocs);
    qq = qrels_get_qid_qrels(rbp->qrels, qid);
    assert(qq != NULL);
    d = rbp->tie_pos;
    actual_depth = MIN(num_scores, depth);
    if (actual_depth == rbp->depth) {
        goto END;
    }

    do {
        unsigned p;
        rel_t rel;
        double tie_frac;

        if (rbp->tie_len == 0) {
            unsigned t;

            rbp->tie_len = 0;
            for (p = 0; p < rbp->persist->p_num; p++) {
                rbp->vals[p].tie_wgt = 0.0;
            }
            /* search forward to see how many documents have this
             * same rank or score.  Note that by starting t at d,
             * we ensure a tie length of always at least 1. */
            for (t = d; t < num_scores; t++) {
                if ( (rbp->ord == QDOCS_ORD_RANK && doc_scores[d].rank
                      != doc_scores[t].rank) || 
                  (rbp->ord == QDOCS_ORD_SCORE && doc_scores[d].score !=
                   doc_scores[t].score) ||
                  (rbp->ord == QDOCS_ORD_OCCUR && doc_scores[d].occur
                   != doc_scores[t].occur)) {
                    break;
                }
                for (p = 0; p < rbp->persist->p_num; p++) {
                    rbp->vals[p].tie_wgt += rbp->vals[p].wgt;
                    rbp->vals[p].wgt *= rbp->persist->p[p];
                }
            }
            assert(t > d);
            rbp->tie_len = t - d;
            for (p = 0; p < rbp->persist->p_num; p++) {
                rbp->vals[p].tie_wgt /= rbp->tie_len;
            }
        }
        assert(rbp->tie_len > 0);
        tie_frac = (double) (MIN(rbp->tie_pos + rbp->tie_len, depth) -
          MAX(rbp->tie_pos, rbp->depth)) / rbp->tie_len;
        for (d = rbp->tie_pos; d < rbp->tie_pos + rbp->tie_len; d++) {
            rel = qid_qrels_get_rel(qq, doc_scores[d].docid);
            if (rel != REL_UNJUDGED)
                rbp->num_rel_ret += (rel * tie_frac);
            for (p = 0; p < rbp->persist->p_num; p++) {
                if (rel == REL_UNJUDGED) {
                    rbp->vals[p].cumerr += rbp->vals[p].tie_wgt * tie_frac;
                } else {
                    rbp->vals[p].sum += rbp->vals[p].tie_wgt * rel * tie_frac;
                }
            }
        }
        /* Does the tie end at or before the depth we're calculating to? */
        if (d <= actual_depth) {
            rbp->tie_pos = d;
            rbp->tie_len = 0;
        }
    } while (d < actual_depth);
    for (p = 0; p < rbp->persist->p_num; p++) {
        rbp->vals[p].err = rbp->vals[p].cumerr
            + pow(rbp->persist->p[p], actual_depth);
    }
    rbp->depth = depth;
END:
    if (num_rel_ret != NULL)
        *num_rel_ret = rbp->num_rel_ret;
    return rbp->vals;
}

void rbp_weights(double * wgts, double persist, unsigned depth) {
    unsigned d;
    double wgt = 1 - persist;
    for (d = 0; d < depth; d++) {
        wgts[d] = wgt;
        wgt *= persist;
    }
}

double rbp(double persist, unsigned rank) {
    return (1 - persist) * pow(persist, rank);
}

void rbp_delete(rbp_t ** rbp_p) {
    rbp_t * rbp;

    rbp = *rbp_p;
    free(rbp->vals);
    free(rbp);
}

#ifdef RBP_MAIN

#define TEST_DEPTH 10000
#define TEST_PERSIST 0.7
#define RESIDUE_MARGIN 0.0001

int main(void) {
    double wgts[TEST_DEPTH];
    double sum = 0.0;
    unsigned d;

    rbp_weights(wgts, TEST_PERSIST, TEST_DEPTH);
    for (d = 0; d < TEST_DEPTH; d++) {
        if (d > 0) {
            assert(wgts[d] = wgts[d - 1] * TEST_PERSIST);
        }
        assert(fabs(rbp(TEST_PERSIST, d) - wgts[d]) < RESIDUE_MARGIN);
        sum += wgts[d];
    }
    assert(sum <= 1.0);
    assert(1.0 - sum < RESIDUE_MARGIN);
    return 0;
}

#endif /* RBP_MAIN */
