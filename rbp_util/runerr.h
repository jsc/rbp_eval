#ifndef RUNERR_H
#define RUNERR_H

#include "run.h"
#include "dococcur.h"
#include "array.h"
#include "strid.h"
#include "qrels.h"
#include "stats.h"

/* FIXME should really dynamically determine this, but... */
#define MAX_NUM_QIDS 1024

enum weight_t {
    WGT_UNIFORM,    /* uniform weighting (that is, unweighted) */
    WGT_LINEAR,     /* area under f(x) [r .. e] */
    WGT_QUADRATIC,  /* area under f(x^2) [r .. e ] */
    WGT_PROJECTED,  /* multiply by projected RBP value */
    WGT_RESIDUAL,   /* multiply by error residual */
    WGT_RESIDUAL_AND_MIDPOINT,    /* by error residual and range midpoint */
    WGT_RESIDUAL_AND_MIDPOINT_SQ, /* square the midpoint */
    WGT_RESIDUAL_AND_MIDPOINT_CB, /* cube the midpoint */
    WGT_RESIDUAL_AND_PROJECTED,   /* by error residual and projected RBP */
    WGT_RESIDUAL_AND_PROJECTED_SQ, /* square the projected */
    WGT_MIDPOINT_SQ  /* squared midpoint rbp */
};

enum query_weight_t {
    WGT_QRY_UNIFORM,
    WGT_QRY_LINEAR   /* multiply by proportion relevance of query */ 
};

enum signif_mode_t {
    SIG_MODE_BASE,         /* base to base */
    SIG_MODE_CONSERVATIVE, /* base to top, naive */
    SIG_MODE_PESSIMAL,     /* base to top, factor out common docs */
    SIG_MODE_PROJECTED     /* base to projected RBP value */
};

struct runinfo {
    run_t * run;
    unsigned rund;
    double rbp;  /* average rbp value */
    double err;  /* average residual error */
    double rbps[MAX_NUM_QIDS];   /* rbp per query */
    double errs[MAX_NUM_QIDS];   /* error per query */
    unsigned judged_depth; /* used in minmaxerr */
    int contributes_judgments; /* does this run contribute to judgment pool? */
};

ARRAY_TYPE_DECL(runarr_t, struct runinfo);

struct qryinfo {
    const char * qid;
    unsigned qidd;
    unsigned judged;
    double rel;
};

ARRAY_TYPE_DECL(qryarr_t, struct qryinfo);

/* Calculating residual error for a collection of runs as documents
 * are incrementally judged. */

typedef struct runerr runerr_t;

runerr_t * new_runerr(double persist, strid_t * qidid, unsigned depth);

void runerr_set_max_num_judgments(runerr_t * runner, 
  unsigned max_num_judgments);

void runerr_set_qrels(runerr_t * runerr, qrels_t * qrels);

void runerr_set_rel_if_unjudged(runerr_t * runerr, double rel);

int runerr_chart(runerr_t * runerr, char * chart_dir, unsigned char_depth,
  unsigned chart_judgments);

void runerr_log_judgments(runerr_t * runerr, FILE * fp);

void runerr_log_scores(runerr_t * runerr, FILE * fp, unsigned interval);

void runerr_log_lacking_documents(runerr_t * runner, FILE * fp);

void runerr_log_significance(runerr_t * runerr, FILE * fp, 
  paired_test_p_fn_t sig_fn, void * sig_fn_data,
  enum signif_mode_t significance_mode, double signif_p_threshold, 
  unsigned interval, double signif_proportion);

void runerr_delete(runerr_t ** runerr_p);

int runerr_load_runs_from_fnames(runerr_t * runerr,
  char ** fnames, unsigned num_files, char * err_buf, unsigned err_buf_size);

int runerr_set_run_noncontributing(runerr_t * runerr, const char * runname);

int runerr_doc_judged(runerr_t * runerr, const char * docid, unsigned qidd,
  unsigned num_qids);

dococcur_item_array_t * runerr_get_dococcur_items(runerr_t * runerr,
  const char * docid, unsigned qidd);

runarr_t * runerr_get_runs(runerr_t * runerr);

double runerr_stats(runerr_t * runerr, double * stddev, double * max,
  unsigned * max_run_d, unsigned * num_judged, double * num_relevant,
  unsigned * num_lacking_judgments);

double runerr_stats_wgt(runerr_t * runerr, double * stddev, double * max,
  unsigned * max_run_d, unsigned * num_judged, double * num_relevant,
  enum weight_t weighting, unsigned * num_lacking_judgments);

double runerr_prop_err_wgt_for_run(runerr_t * runerr, unsigned rund,
  enum weight_t weighting);

double runerr_wgt_for_qry(runerr_t * runerr, unsigned qidd, 
  enum query_weight_t weighting);

/*
 * Find the maximum weighted error as a proportional of total error.
 */
double runerr_max_prop_wgt(runerr_t * runerr, unsigned * max_rund_d,
  enum weight_t weighting);

/*
 *  Proportion of run rbp differences that are significant.
 *
 *  A one-tailed paired Wilcoxon test is used.  Scores are based
 *  on rbp alone; error bounds are ignored.
 */
double runerr_significance(runerr_t * runerr, double p_threshold,
  double top_proportion, paired_test_p_fn_t sig_fn, void * sig_fn_data,
  enum signif_mode_t signif_mode);

void runerr_stop_when_top_run_found(runerr_t * runerr);

unsigned runerr_num_judged(runerr_t * runerr);

/*
 *  Proportion of run rbp differences that are conservatively significant.
 *
 *  A one-tailed paired sign test is used, comparing the lower-bound
 *  rbps of the better run against the upper-bound rbps (rbp + err)
 *  of the worse run.
 */
/* double runerr_significance_conservative(runerr_t * runerr, double p);

double runerr_top_significance_conservative(runerr_t * runerr, double p,
  double top_proportion); */

#endif /* RUNERR_H */
