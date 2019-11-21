#ifndef COMMON_H
#define COMMON_H

/* Routines that are common to a number of utility programs without
 * having a distinct module that they fit into. */

#include <stdio.h>
#include "array.h"
#include "runerr.h"
#include "strid.h"

#define MAX_DEPTH 10000

#define DEFAULT_PERSIST 0.95

#define DEFAULT_SCORE_LOG_INTERVAL 100

#define DEFAULT_SIGNIFICANCE_LOG_INTERVAL 100
#define DEFAULT_SIGNIFICANCE_FUNCTION paired_wilcoxon_test_p
#define DEFAULT_SIGNIFICANCE_MODE SIG_MODE_PESSIMAL
#define DEFAULT_SIGNIFICANCE_PROPORTION 1.0

#define DEFAULT_SIGNIFICANCE_P_THRESHOLD 0.05

#define DEFAULT_MAX_NUM_JUDGMENTS UINT_MAX

#define COMMON_OPTS "p:C:s:S:Q:E:j:Z:z:P:m:J:N:TG:L:U:"

ARRAY_TYPE_DECL(uint_arr_t, unsigned);
ARRAY_TYPE_DECL(str_arr_t, char *);

struct common {
    double persist;
    double min_avg_err;
    char * chart_dir;
    unsigned score_log_interval;
    FILE * score_log_fp;
    FILE * judgment_log_fp;
    char * qrels_fname;
    FILE * signif_log_fp;
    FILE * lacking_log_fp;
    paired_test_p_fn_t signif_fn;
    unsigned signif_log_interval;
    enum signif_mode_t signif_mode;
    double signif_proportion;
    unsigned max_depth;
    unsigned max_num_judgments;
    double rel_if_unjudged;

    strid_t * qidid;
    str_arr_t non_contrib_runs;
    int stop_when_top_run_found;
};

/* Load depths to which you wish reports to be made from a file.
 * The depths are specified a line per depth, in ascending order. */
int load_report_depths(char * fname, uint_arr_t * depths);

runerr_t * init_runerr(struct common * c, char ** run_fnames, unsigned num_run_fnames);

double report_runerr_stats(FILE * fp, runerr_t * runerr, unsigned * num_judged,
  double * num_relevant);

void common_init(struct common * c);

int common_process_option(struct common * c, int optflag, char * optarg);

void common_cleanup(struct common * c);

#endif /* COMMON_H */
