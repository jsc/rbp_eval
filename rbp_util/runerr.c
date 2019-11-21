#include <math.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include "array.h"
#include "util.h"
#include "runerr.h"
#include "dococcur.h"
#include "rbp.h"
#include "qdocs.h"
#include "stats.h"
#include "sign.h"
#include "strhash.h"
#include "dqhash.h"

/* use results from documents judged so far to project an RBP
 * value. */
#define PROJECTED_RBP(r, e) (((r) + ((r) * (e)) / (1.0 - (e))))

/* Prevent a run from getting no weight in the WGT_PROJECTED weighting
 * scheme because the first few documents examined are all irrelevant;
 * otherwise, its projected RBP score is 0.0. */
#define WGT_ERR_PROJ_MIN 0.01

/*
 * Minimum weighting a query can have.
 *
 * This prevents a query being starved of judgments if so far
 * no relevant documents have been found.
 */
#define WGT_QUERY_MIN 0.01

double projected_rbp(double base, double residual) {
    if (residual == 1.0) {
        return 1.0;
    } else {
        return PROJECTED_RBP(base, residual);
    }
}

struct runerr {
    runarr_t runs;
    qryarr_t queries;

    dococcur_t * dcr;
    double * rbp_wgts;
    unsigned rbp_wgts_len;
    double persist;
    unsigned num_judged;
    strid_t * qidid;
    double num_relevant;
    unsigned depth;
    unsigned max_num_judgments;
    unsigned lacking_judgments;
    double rel_if_unjudged;

    /* charting info. */
    char * chart_dir;
    unsigned chart_depth;
    unsigned chart_judgments;

    /* log rbp scores */
    FILE * score_log_fp;
    unsigned score_log_interval;

    /* log judgments */
    FILE * judgment_log_fp;

    /* log judgments lacking */
    FILE * lacking_judgments_log_fp;

    /* log significance results */
    FILE * signif_log_fp;
    paired_test_p_fn_t signif_fn;
    void * signif_fn_data;
    enum signif_mode_t signif_mode;
    double signif_p_threshold;
    unsigned signif_log_interval;
    double signif_proportion;

    /* judgment methods that involve qrels */
    qrels_t * qrels;

    /* record which doc/qids have been judged. */
    strhash_t * docs_judged;

    /* do we stop when the top-ranking run has been identified? */
    int stop_when_top_run_found;
};

static void _runerr_add_run(runerr_t * runerr, run_t * run);

static int _runerr_chart(runerr_t * runerr, const char * docid, unsigned qidd,
  unsigned num_qids);

static void _runerr_log_scores(runerr_t * runerr);

runerr_t * new_runerr(double persist, strid_t * qidid, unsigned depth) {
    runerr_t * runerr;
    unsigned num_qids, q;

    runerr = util_malloc_or_die(sizeof(*runerr));
    runerr->dcr = new_dococcur();
    runerr->persist = persist;
    runerr->num_judged = 0;
    runerr->qidid = qidid;
    runerr->depth = depth;
    runerr->max_num_judgments = UINT_MAX;
    runerr->lacking_judgments = 0;

    runerr->rbp_wgts = util_malloc_or_die(sizeof(*runerr->rbp_wgts)
      * runerr->depth);
    rbp_weights(runerr->rbp_wgts, runerr->persist, runerr->depth);

    runerr->num_relevant = 0;

    runerr->chart_dir = NULL;
    runerr->qrels = NULL;

    runerr->score_log_fp = NULL;
    ARRAY_INIT(runerr->runs);
    ARRAY_INIT(runerr->queries);

    runerr->judgment_log_fp = NULL;
    runerr->docs_judged = new_strhash();

    runerr->signif_log_fp = NULL;
    runerr->signif_fn = NULL;
    runerr->signif_fn_data = NULL;
    runerr->signif_mode = -1;
    runerr->signif_log_interval = -1;

    runerr->stop_when_top_run_found = 0;

    runerr->rel_if_unjudged = 0.0;

    num_qids = strid_num_ids(runerr->qidid);
    assert(num_qids > 0);
    for (q = 0; q < num_qids; q++) {
        struct qryinfo qi;
        qi.qid = strid_get_str(runerr->qidid, q);
        qi.qidd = q;
        qi.judged = 0;
        qi.rel = 0.0;
        ARRAY_ADD(runerr->queries, qi);
    }

    return runerr;
}

void runerr_set_qrels(runerr_t * runerr, qrels_t * qrels) {
    runerr->qrels = qrels;
}

void runerr_set_rel_if_unjudged(runerr_t * runerr, double rel) {
    runerr->rel_if_unjudged = rel;
}

void runerr_set_max_num_judgments(runerr_t * runerr, 
  unsigned max_num_judgments) {
    runerr->max_num_judgments = max_num_judgments;
}

void runerr_log_scores(runerr_t * runerr, FILE * fp, unsigned interval) {
    runerr->score_log_fp = fp;
    runerr->score_log_interval = interval;
}

void runerr_log_judgments(runerr_t * runerr, FILE * fp) {
    runerr->judgment_log_fp = fp;
}

void runerr_log_lacking_documents(runerr_t * runerr, FILE * fp) {
    runerr->lacking_judgments_log_fp = fp;
}

void runerr_stop_when_top_run_found(runerr_t * runerr) {
    runerr->stop_when_top_run_found = 1;
}

void runerr_log_significance(runerr_t * runerr, FILE * fp,
  paired_test_p_fn_t signif_fn, void * signif_fn_data,
  enum signif_mode_t signif_mode, double signif_p_threshold, 
  unsigned interval, double signif_proportion) {
    runerr->signif_log_fp = fp;
    runerr->signif_fn = signif_fn;
    runerr->signif_fn_data = signif_fn_data;
    runerr->signif_mode = signif_mode;
    runerr->signif_p_threshold = signif_p_threshold;
    runerr->signif_log_interval = interval;
    runerr->signif_proportion = signif_proportion;
}

void runerr_delete(runerr_t ** runerr_p) {
    runerr_t * runerr = *runerr_p;
    unsigned r;

    dococcur_delete(&runerr->dcr);
    if (runerr->chart_dir)
        free(runerr->chart_dir);
    if (runerr->qrels)
        qrels_delete(&runerr->qrels);
    for (r = 0; r < runerr->runs.elem_count; r++) {
        if (runerr->runs.elems[r].run) {
            run_delete(&runerr->runs.elems[r].run);
        }
    }
    free(runerr->rbp_wgts);
    ARRAY_DELETE(runerr->runs);
    ARRAY_DELETE(runerr->queries);
    if (runerr->docs_judged)
        strhash_delete(&runerr->docs_judged, NULL);
    free(runerr);
    *runerr_p = NULL;
}

int runerr_chart(runerr_t * runerr, char * chart_dir, unsigned chart_depth,
  unsigned chart_judgments) {
    struct stat st_buf;
    if (stat(chart_dir, &st_buf) < 0) {
        return -1;
    }
    if (!S_ISDIR(st_buf.st_mode)) {
        errno = ENOTDIR;
        return -1;
    }
    /* could check permissions here but I'm too lazy. */
    runerr->chart_dir = util_strdup_or_die(chart_dir);
    runerr->chart_depth = chart_depth;
    runerr->chart_judgments = chart_judgments;
    return 0;
}

#define INNER_ERR_BUF_SIZE 1024

int runerr_load_runs_from_fnames(runerr_t * runerr,
  char ** fnames, unsigned num_files, char * err_buf, unsigned err_buf_size) {
    unsigned r;
    char inner_err_buf[INNER_ERR_BUF_SIZE];
    for (r = 0; r < num_files; r++) {
        run_t * run;
        char * fname;
        FILE * fp;

        fname = fnames[r];
        fp = fopen(fname, "r");
        if (fp == NULL) {
            snprintf(err_buf, err_buf_size, 
              "Unable to open run file '%s' for reading: %s\n",
              fname, strerror(errno));
            return -1;
        }
        run = load_run(fp, inner_err_buf, INNER_ERR_BUF_SIZE);
        fclose(fp);
        if (run == NULL) {
            snprintf(err_buf, err_buf_size, "Error loading run file '%s': %s\n",
              fname, inner_err_buf);
            return -1;
        }
        _runerr_add_run(runerr, run);
    }
    return 0;
}

static void _runerr_add_run(runerr_t * runerr, run_t * run) {
    struct runinfo ri;
    unsigned q;
    strid_t * qidid = runerr->qidid;
    ri.run = run;
    ri.err = 1.0;
    ri.rbp = 0.0;
    ri.judged_depth = 0;
    ri.rund = runerr->runs.elem_count;
    ri.contributes_judgments = 1;
    for (q = 0; q < MAX_NUM_QIDS; q++) {
        ri.rbps[q] = 0.0;
        ri.errs[q] = 1.0;
    }
    dococcur_add_run(runerr->dcr, run, runerr->runs.elem_count, qidid);
    ARRAY_ADD(runerr->runs, ri);
}

int runerr_set_run_noncontributing(runerr_t * runerr, const char * runid) {
    unsigned r;
    for (r = 0; r < runerr->runs.elem_count; r++) {
        struct runinfo * ri = &runerr->runs.elems[r];
        if (strcmp(run_get_runid(ri->run), runid) == 0) {
            ri->contributes_judgments = 0;
            return 1;
        }
    }
    return 0;
}

int runerr_doc_judged(runerr_t * runerr, const char * docid, unsigned qidd,
  unsigned num_qids) {
    unsigned i;
    dococcur_item_array_t * dococ;
    double rel = -1.0;
    int is_judged = 0;
    const char * qid = strid_get_str(runerr->qidid, qidd);
    int finished = 0;

    dococ = dococcur_get(runerr->dcr, docid, qidd);
    assert(dococ != NULL);
    runerr->queries.elems[qidd].judged++;
    if (runerr->qrels) {
        rel = qrels_get_rel(runerr->qrels, qid, docid);
        if (rel < 0.0) {
            runerr->lacking_judgments++;
            if (runerr->lacking_judgments_log_fp) {
                FILE * fp = runerr->lacking_judgments_log_fp;
                int i;
                fprintf(fp, "%s %d:", docid, qidd);
                for (i = 0; i < dococ->elem_count; i++) {
                    dococcur_item_t di;
                    struct runinfo * ri;

                    di = dococ->elems[i];
                    ri = &runerr->runs.elems[di.rund];
                    fprintf(fp, " (%s, %d)", run_get_runid(ri->run), di.rank);
                }
                fprintf(fp, "\n");
            } else {
                fprintf(stderr, "Unjudged: %s %d: %u\n", docid, qidd,
                  runerr->num_judged);
            }
            rel = runerr->rel_if_unjudged;
        } else {
            is_judged = 1;
        }
        runerr->queries.elems[qidd].rel += rel;
        if (rel > 0.0)
            runerr->num_relevant += rel;
    }
    if (runerr->judgment_log_fp) {
        double rep_rel;
        if (is_judged)
            rep_rel = rel;
        else
            rep_rel = -1.0;
        fprintf(runerr->judgment_log_fp, "%s %s %.2lf\n", qid, docid, rep_rel);
    }
    if (runerr->docs_judged) {
        strhash_data_t * data;
        int found = 0;
        data = dqhash_update(runerr->docs_judged, docid, qidd, &found);
        assert(found == 0);
        data->lf = rel;
    }

    for (i = 0; i < dococ->elem_count; i++) {
        dococcur_item_t di;
        double wgt;
        doc_score_t * ds;
        struct runinfo * ri;

        di = dococ->elems[i];
        ds = di.data;
        ds->flags = 1;
        assert(di.rank < runerr->depth);
        ri = &runerr->runs.elems[di.rund];
        wgt = runerr->rbp_wgts[di.rank];
        ri->err -= wgt / num_qids;
        ri->errs[qidd] -= wgt;
        if (rel > 0.0) {
            ri->rbp += (wgt * rel) / num_qids;
            ri->rbps[qidd] += (rel * wgt);
        }
    }
    if (runerr->chart_dir != NULL && runerr->num_judged < 
      runerr->chart_judgments) {
        _runerr_chart(runerr, docid, qidd, num_qids);
    }
    runerr->num_judged++;
    if (runerr->score_log_fp != NULL && (runerr->num_judged %
      runerr->score_log_interval) == 0) {
        _runerr_log_scores(runerr);
    }
    if (runerr->signif_log_fp != NULL && (runerr->num_judged %
          runerr->signif_log_interval) == 0) {
        double signif_pc;

        signif_pc = runerr_significance(runerr, runerr->signif_p_threshold,
          runerr->signif_proportion, runerr->signif_fn, runerr->signif_fn_data, 
          runerr->signif_mode);
        fprintf(runerr->signif_log_fp, "%u %.4lf\n", runerr->num_judged,
          signif_pc);
    }
    if (runerr->stop_when_top_run_found) {
        unsigned r;
        double max_base = 0.0;
        unsigned max_base_pos = 0;
        int distinct_top = 1;
        for (r = 0; r < runerr->runs.elem_count; r++) {
            struct runinfo * ri = &runerr->runs.elems[r];
            if (ri->rbp > max_base) {
                max_base = ri->rbp;
                max_base_pos = r;
            }
        }
        for (r = 0; distinct_top && r < runerr->runs.elem_count; r++) {
            struct runinfo * ri = &runerr->runs.elems[r];
            if (r == max_base_pos)
                continue;
            if (ri->rbp + ri->err >= max_base)
                distinct_top = 0;
        }
        if (distinct_top) {
            finished = 1;
            if (runerr->score_log_fp) {
                _runerr_log_scores(runerr);
            } else {
                fprintf(stdout, "# Top run found after %u judgments\n",
                  runerr->num_judged);
            }
        }
    }
    if (runerr->num_judged >= runerr->max_num_judgments) {
        finished = 1;
    }
    return !finished;
}

dococcur_item_array_t * runerr_get_dococcur_items(runerr_t * runerr,
  const char * docid, unsigned qidd) {
    return dococcur_get(runerr->dcr, docid, qidd);
}

runarr_t * runerr_get_runs(runerr_t * runerr) {
    return &runerr->runs;
}

double runerr_stats(runerr_t * runerr, double * stddev, double * max,
  unsigned * max_run_d, unsigned * num_judged, double * num_relevant,
  unsigned * num_lacking_judgments) {
    return runerr_stats_wgt(runerr, stddev, max, max_run_d, num_judged, 
      num_relevant, WGT_UNIFORM, num_lacking_judgments);
}

/* NOTE these formulas (somewhat confusingly) give the unaveraged weight,
 * that is, the weight of the run's error bound; they need to be divided by 
 * the error of the run to get the weighting that is to be multiplied by
 * a document's error bound to get the weighting for that document. */

#define WGT_ERR_UNIFORM(r, e) (e)
#define WGT_ERR_LINEAR(r, e) ((0.5 * (pow((r) + (e), 2) - pow((r), 2))))
#define WGT_ERR_QUADRATIC(r, e) ((0.333333 * (pow((r) + (e), 3) - pow((r), 3))))
#define WGT_ERR_PROJ(r, e) (((r) + (((r) * (e)) / (1.0 - (e)))) * (e))
#define WGT_ERR_RESIDUAL(r, e) ((e) * (e))
#define WGT_ERR_RESIDUAL_AND_MIDPOINT(r, e) ((r + (e / 2)) * (e) * (e))
/* #define WGT_ERR_RESIDUAL_AND_PROJ(r, e) (((r) + (((r) * (e)) / (1.0 - (e)))) * (e) * (e)) */
#define WGT_ERR_RESIDUAL_AND_PROJ(r, e) ((PROJECTED_RBP((r), (e))) * (e) * (e))
#define WGT_ERR_RESIDUAL_AND_MIDPOINT_SQ(r, e) \
    (pow((r + (e / 2)), 2) * (e) * (e))
#define WGT_ERR_RESIDUAL_AND_PROJ_SQ(r, e) \
    (pow(PROJECTED_RBP((r), (e)), 2) * (e) * (e))
#define WGT_ERR_RESIDUAL_AND_MIDPOINT_CB(r, e) \
    (pow((r + (e / 2)), 3) * (e) * (e))
#define WGT_ERR_MIDPOINT_SQ(r, e) (((r) + (e) / 2) * ((r) + (e) / 2) * (e))

double wgt_err(double rbp, double err, enum weight_t weighting) {
    switch (weighting) {
    case WGT_UNIFORM:
        return WGT_ERR_UNIFORM(rbp, err);
    case WGT_LINEAR:
        return WGT_ERR_LINEAR(rbp, err);
    case WGT_QUADRATIC:
        return WGT_ERR_QUADRATIC(rbp, err);
    case WGT_PROJECTED:
        if (err == 1.0) {
            return err;
        } else {
            double wgt = WGT_ERR_PROJ(rbp, err);
            if (wgt / err < WGT_ERR_PROJ_MIN)
                return WGT_ERR_PROJ_MIN;
            else
                return wgt;
        }
    case WGT_RESIDUAL:
        return WGT_ERR_RESIDUAL(rbp, err);
    case WGT_RESIDUAL_AND_MIDPOINT:
        return WGT_ERR_RESIDUAL_AND_MIDPOINT(rbp, err);
    case WGT_RESIDUAL_AND_MIDPOINT_SQ:
        return WGT_ERR_RESIDUAL_AND_MIDPOINT_SQ(rbp, err);
    case WGT_RESIDUAL_AND_MIDPOINT_CB:
        return WGT_ERR_RESIDUAL_AND_MIDPOINT_CB(rbp, err);
    case WGT_RESIDUAL_AND_PROJECTED:
        return WGT_ERR_RESIDUAL_AND_PROJ(rbp, err);
    case WGT_RESIDUAL_AND_PROJECTED_SQ:
        return WGT_ERR_RESIDUAL_AND_PROJ_SQ(rbp, err);
    case WGT_MIDPOINT_SQ:
        return WGT_ERR_MIDPOINT_SQ(rbp, err);
    default:
        assert(0);
        return -1.0;
    }
}

double runerr_prop_err_wgt_for_run(runerr_t * runerr, unsigned rund,
  enum weight_t weighting) {
    struct runinfo * ri = &runerr->runs.elems[rund];
    return wgt_err(ri->rbp, ri->err, weighting) / (ri->err);
}

double runerr_wgt_for_qry(runerr_t * runerr, unsigned qidd, 
  enum query_weight_t weighting) {
    struct qryinfo * qi = &runerr->queries.elems[qidd];
    switch (weighting) {
    case WGT_QRY_UNIFORM:
        return 1.0;
    case WGT_QRY_LINEAR:
        return WGT_QUERY_MIN + qi->rel / qi->judged;
    default:
        assert(0);
        return -1.0;
    }
}

double runerr_max_prop_wgt(runerr_t * runerr, unsigned * max_run_d,
  enum weight_t weighting) {
    unsigned r;
    double max = 0.0;
    for (r = 0; r < runerr->runs.elem_count; r++) {
        struct runinfo * ri;
        double err;
        ri = &runerr->runs.elems[r];
        err = wgt_err(ri->rbp, ri->err, weighting) / (ri->err);
        if (err > max) {
            max = err;
            *max_run_d = r;
        }
    }
    return max;
}

double runerr_stats_wgt(runerr_t * runerr, double * stddev, double * max,
  unsigned * max_run_d, unsigned * num_judged, double * num_relevant, 
  enum weight_t weighting, unsigned * num_lacking_judgments) {
    unsigned r;
    double tot_err = 0.0;
    double avg_err;
    double var = 0.0;

    *max = 0.0;
    for (r = 0; r < runerr->runs.elem_count; r++) {
        double err;
        struct runinfo * ri;
        ri = &runerr->runs.elems[r];
        err = wgt_err(ri->rbp, ri->err, weighting);
        tot_err += err;
        if (err > *max) {
            *max = err;
            *max_run_d = r;
        }
    }
    avg_err = tot_err / runerr->runs.elem_count;
    for (r = 0; r < runerr->runs.elem_count; r++) {
        double v, err;
        struct runinfo * ri;
        ri = &runerr->runs.elems[r];
        err = wgt_err(ri->rbp, ri->err, weighting);
        v = (avg_err - err);
        var += v * v;
    }
    /* XXX as this is a sample, should strictly speaking divide
     * by (n - 1), but since the divisor will be the same for
     * all runs, doesn't make a difference here. */
    var /= runerr->runs.elem_count;
    *stddev = sqrt(var);
    *num_judged = runerr->num_judged;
    *num_relevant = runerr->num_relevant;
    if (num_lacking_judgments) {
        *num_lacking_judgments = runerr->lacking_judgments;
    }
    return avg_err;
}

struct rbp_id {
    double rbp;
    unsigned id;
};

static int rbp_id_cmp(const void * a, const void * b) {
    struct rbp_id * ra = (struct rbp_id *) a;
    struct rbp_id * rb = (struct rbp_id *) b;
    if (ra->rbp > rb->rbp) {
        return -1;  /* descending order. */
    } else if (ra->rbp < rb->rbp) {
        return 1;
    } else {
        return 0;
    }
}

unsigned runerr_pessimal_significance_pairs(runerr_t * runerr, 
  double p_threshold, struct runinfo * run_a, struct runinfo ** runs_b, 
  unsigned num_qids, unsigned num_runs_b, paired_test_p_fn_t sig_fn, 
  void * sig_fn_data, unsigned depth) {
    double * rbp_a, * rbp_b, * err_b;
    unsigned q, r;
    double * wrk_space;
    unsigned * rund_to_b_ind;
    unsigned asl;

    rund_to_b_ind = util_malloc_or_die(sizeof(*rund_to_b_ind) * runerr->runs.elem_count);
    for (r = 0; r < runerr->runs.elem_count; r++) {
        rund_to_b_ind[r] = UINT_MAX;
    }
    for (r = 0; r < num_runs_b; r++) {
        rund_to_b_ind[runs_b[r]->rund] = r;
    }
    wrk_space = util_malloc_or_die(sizeof(double) * num_runs_b * num_qids * 3);
    rbp_a = wrk_space;
    rbp_b = wrk_space + (num_runs_b * num_qids);
    err_b = wrk_space + (num_runs_b * num_qids * 2);
    for (r = 0; r < num_runs_b * num_qids * 3; r++) {
        wrk_space[r] = -1.0;
    }
    for (r = 0; r < num_runs_b; r++) {
        for (q = 0; q < num_qids; q++) {
            unsigned i = r * num_qids + q;
            struct runinfo * run_b = runs_b[r];

            assert(rbp_a[i] == -1.0);
            assert(rbp_b[i] == -1.0);
            assert(err_b[i] == -1.0);
            assert(run_a->rbps[q] >= -0.001);
            assert(run_b->rbps[q] >= -0.001);
            assert(run_b->errs[q] >= -0.001);
            rbp_a[i] = run_a->rbps[q];
            rbp_b[i] = run_b->rbps[q];
            err_b[i] = run_b->errs[q];
        }
    }
    for (q = 0; q < num_qids; q++) {
        char * qid;
        qdocs_t * qdocs;
        struct doc_score * scores;
        unsigned d;
        unsigned num_scores;
        double judged_rbp = 0.0;

        qid = strid_get_str(runerr->qidid, q);
        qdocs = run_get_qdocs_by_qid(run_a->run, qid);
        assert(qdocs != NULL);
        num_scores = qdocs_num_scores(qdocs);
        if (num_scores > runerr->depth)
            num_scores = runerr->depth;
        scores = qdocs_get_scores(qdocs, QDOCS_DEFAULT_ORDERING);
        for (d = 0; d < num_scores; d++) {
            char * docid = scores[d].docid;
            int judged;
            strhash_data_t data;

            data = dqhash_get(runerr->docs_judged, docid, q, &judged);
            if (!judged) {
                double a_wgt = runerr->rbp_wgts[d];
                dococcur_item_array_t * dococ;
                unsigned o;
                int seen_here = 0;

                dococ = dococcur_get(runerr->dcr, docid, q);
                assert(dococ != NULL);
                for (o = 0; o < dococ->elem_count; o++) {
                    dococcur_item_t * di;
                    double b_wgt;
                    unsigned b_ind;

                    di = &dococ->elems[o];
                    if (di->rund == run_a->rund) {
                        assert(seen_here == 0);
                        seen_here = 1;
                    }
                    b_ind = rund_to_b_ind[di->rund];
                    if (b_ind != UINT_MAX) {
                        unsigned i = b_ind * num_qids + q;
                        b_wgt = runerr->rbp_wgts[di->rank];
                        err_b[i] -= b_wgt;
                        assert(err_b[i] >= -0.001);

                        if (a_wgt < b_wgt) {
                            assert(d > di->rank);
                            rbp_b[i] += b_wgt;
                            rbp_a[i] += a_wgt;
                            assert(rbp_b[i] <= 1.001);
                            assert(rbp_a[i] <= 1.001);
                        }
                    }
                }
                assert(seen_here == 1);
            } else {
                if (data.lf > 0.0)
                    judged_rbp += (runerr->rbp_wgts[d] * data.lf);
            }
        }
        assert(fabs(judged_rbp - run_a->rbps[q]) < 0.001);
        for (r = 0; r < num_runs_b; r++) {
            unsigned i = r * num_qids + q;
            rbp_b[i] += err_b[i];
            assert(rbp_b[i] <= 1.001);
        }
    }

#ifndef NDEBUG
    for (r = 0; r < num_runs_b; r++) {
        for (q = 0; q < num_qids; q++) {
            unsigned i = r * num_qids + q;
            struct runinfo * run_b = runs_b[r];

            assert(rbp_a[i] >= run_a->rbps[q] - 0.001);
            assert(rbp_a[i] <= 1.001);
            assert(rbp_b[i] >= run_b->rbps[q] - 0.001);
            assert(rbp_b[i] <= 1.001);
            assert(err_b[i] >= -0.001);
            assert(err_b[i] <= run_b->errs[q] + 0.001);
            assert(rbp_a[i] <= run_a->rbps[q] + run_a->errs[q] + 0.001);
            assert(rbp_b[i] <= run_b->rbps[q] + run_b->errs[q] + 0.001);
            assert(rbp_a[i] - run_a->rbps[q] < rbp_b[i] - run_b->rbps[q] + 0.001);
        }
    }
#endif /* NDEBUG */
    /* calculate significance */
    asl = 0;
    for (r = 0; r < num_runs_b; r++) {
        double p;
        p = sig_fn(rbp_a + (r * num_qids), rbp_b + (r * num_qids), num_qids, sig_fn_data);
        if (p < p_threshold) {
            asl++;
        }
    }

    free(wrk_space);
    free(rund_to_b_ind);
    return asl;
}

double runerr_significance(runerr_t * runerr, double p_threshold, 
  double top_proportion, paired_test_p_fn_t sig_fn, void * sig_fn_data,
  enum signif_mode_t signif_mode) {
    unsigned num_runs = runerr->runs.elem_count;
    unsigned i, j;
    unsigned num_top_runs;
    struct rbp_id * vals = NULL;
    unsigned r;
    double * dist_b = NULL;
    unsigned num_qids = strid_num_ids(runerr->qidid);
    unsigned pairs = 0, asl = 0;
    struct runinfo ** runs_b = NULL;

    assert(top_proportion > 0.0);
    assert(top_proportion <= 1.0);
    num_top_runs = num_runs * top_proportion + 0.5;
    if (num_top_runs > num_runs)
        num_top_runs = num_runs;
    vals = util_malloc_or_die(sizeof(*vals) * num_runs);
    for (r = 0; r < num_runs; r++) {
        vals[r].id = r;
        vals[r].rbp = runerr->runs.elems[r].rbp;
    }
    qsort(vals, num_runs, sizeof(vals[0]), rbp_id_cmp);
    if (signif_mode == SIG_MODE_CONSERVATIVE
      || signif_mode == SIG_MODE_PROJECTED) {
        dist_b = util_malloc_or_die(sizeof(*dist_b) * num_qids);
    } else if (signif_mode == SIG_MODE_PESSIMAL) {
        runs_b = util_malloc_or_die(sizeof(*runs_b) * num_top_runs);
        for (r = 0; r < num_top_runs; r++) {
            runs_b[r] = &runerr->runs.elems[vals[r].id];
        }
    }
    for (i = 0; i < num_top_runs; i++) {
        if (signif_mode == SIG_MODE_PESSIMAL) {
            unsigned num_runs_b = num_top_runs - i - 1;
            pairs += num_runs_b;
            asl += runerr_pessimal_significance_pairs(runerr,
              p_threshold, &runerr->runs.elems[vals[i].id],
              runs_b + i + 1, num_qids, num_runs_b, sig_fn, sig_fn_data,
              runerr->depth);
        } else {
            for (j = i + 1; j < num_top_runs; j++) {
                struct runinfo * ra, * rb;
                double p;
                unsigned s;

                pairs++;
                ra = &runerr->runs.elems[vals[i].id];
                rb = &runerr->runs.elems[vals[j].id];
                assert(ra->rbp >= rb->rbp);
                if (signif_mode == SIG_MODE_CONSERVATIVE) {
                    for (s = 0; s < num_qids; s++) {
                        dist_b[s] = rb->rbps[s] + rb->errs[s];
                    }
                } else if (signif_mode == SIG_MODE_PROJECTED) {
                    for (s = 0; s < num_qids; s++) {
                        dist_b[s] = projected_rbp(rb->rbps[s], rb->errs[s]);
                    }
                } else {
                    dist_b = rb->rbps;
                }
                p = sig_fn(ra->rbps, dist_b, num_qids, sig_fn_data);
                if (p < p_threshold) {
                    asl++;
                }
            }
        }
    }

    free(vals);
    if (signif_mode == SIG_MODE_CONSERVATIVE) {
        free(dist_b);
    }
    if (runs_b != NULL) {
        free(runs_b);
    }
    return (double) asl / pairs;
}

unsigned runerr_num_judged(runerr_t * runerr) {
    return runerr->num_judged;
}

static int _runerr_chart(runerr_t * runerr, const char * docid, unsigned qidd,
  unsigned num_qids) {
    FILE * chart_fp = NULL;
    char chart_fname_buf[PATH_MAX];
    unsigned q;

    snprintf(chart_fname_buf, PATH_MAX, "%s/%u.html", runerr->chart_dir,
      runerr->num_judged);
    chart_fp = fopen(chart_fname_buf, "w");
    if (chart_fp == NULL) {
        fprintf(stderr, "ERROR: unable to open chart file '%s'; "
          "turning charting off\n", chart_fname_buf);
        free(runerr->chart_dir);
        runerr->chart_dir = NULL;
        return -1;
    } else {
        unsigned i;
        fprintf(chart_fp, "<html><head><title>Judgement Chart: %u</title>",
          runerr->num_judged);
        fprintf(chart_fp, "<link rel='stylesheet' href='judgments.css' type='text/css'>\n");
        fprintf(chart_fp, "</head>\n");
        fprintf(chart_fp, "<body><h1>Judgement Chart: %u</h1>\n", 
          runerr->num_judged);
        fprintf(chart_fp, "<div class='nav'><a href='%u.html'>prev</a> | "
          "<a href='%u.html'>next</a></div>\n", runerr->num_judged - 1,
          runerr->num_judged + 1);
        fprintf(chart_fp, "<div class='nav'>\n");
        for (i = 0; i * 20 < runerr->chart_judgments; i++) {
            fprintf(chart_fp, "<a href='%u.html'>%u</a> | \n",
              i * 20, i * 20);
        }
        fprintf(chart_fp, "</div>\n");
        fprintf(chart_fp, "<div class='info'>Judged: docid %s, query %u"
          "</div>\n", docid, qidd);
        for (q = 0; q < num_qids; q++) {
            unsigned r;
            fprintf(chart_fp, "<table>\n");
            for (r = 0; r < runerr->runs.elem_count; r++) {
                unsigned d;
                struct runinfo * ri = &runerr->runs.elems[r];
                run_t * run = ri->run;
                qdocs_t * qd = run_get_qdocs_by_index(run, q);

                fprintf(chart_fp, "<tr>\n");
                fprintf(chart_fp, "<td class='err'>%.4lf</td>\n",
                  ri->err);
                for (d = 0; d < runerr->chart_depth; d++) {
                    /* XXX next assumes qidd and index in run are
                     * the same. */
                    unsigned numranks = qdocs_num_scores(qd);
                    if (d < numranks) {
                        doc_score_t * ds2 = qdocs_get_scores(qd, QDOCS_DEFAULT_ORDERING);
                        if (q == qidd && strcmp(docid, ds2[d].docid) == 0) {
                            fprintf(chart_fp, "<td class='justjudged'>");
                        } else if (ds2[d].flags == 1) {
                            fprintf(chart_fp, "<td class='judged'>");
                        } else {
                            fprintf(chart_fp, "<td class='unjudged'>");
                        }
                    } else {
                        fprintf(chart_fp, "<td class='unranked'>");
                    }
                    fprintf(chart_fp, "</td>");
                }
                fprintf(chart_fp, "</tr>\n");
            }
            fprintf(chart_fp, "</table>\n");
        }
        fprintf(chart_fp, "</html>\n");
        fclose(chart_fp);
    }
    return 0;
}

static void _runerr_log_scores(runerr_t * runerr) {
    if (runerr->score_log_fp != NULL) {
        unsigned r;
        runarr_t * runarr = &runerr->runs;

        fprintf(runerr->score_log_fp, "# %u judged\n", runerr->num_judged);
        for (r = 0; r < runarr->elem_count; r++) {
            struct runinfo * ri = &runarr->elems[r];
            fprintf(runerr->score_log_fp, "%s %.4lf +%.4lf\n",
              run_get_runid(ri->run), ri->rbp, ri->err);
        }
        fprintf(runerr->score_log_fp, "\n");
    }
}
