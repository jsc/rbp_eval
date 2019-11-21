#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include "common.h"
#include "wilcoxon.h"
#include "sign.h"
#include "t.h"
#include "bootstrap.h"

#define LINE_BUF_SIZE 1024

/* FIXME these should be options to init_runerr */
#define DEFAULT_CHART_DEPTH 100
#define DEFAULT_CHART_JUDGMENTS 500

#define ERR_BUF_LEN 1024

#define MAX_NUM_QIDS 1024

runerr_t * init_runerr(struct common * c, char ** run_fnames, unsigned num_run_fnames) {
    runerr_t * runerr;
    qrels_t * qrels = NULL;
    char err_buf[ERR_BUF_LEN];
    const char * qids[MAX_NUM_QIDS];
    unsigned num_qids;
    unsigned r;
    unsigned q;

    if (c->qrels_fname != NULL ) {
        FILE * fp;

        fp = fopen(c->qrels_fname, "r");
        if (fp == NULL) {
            fprintf(stderr, "Unable to open qrels file '%s' for reading: "
              "%s\n", c->qrels_fname, strerror(errno));
            return NULL;
        }
        qrels = load_qrels(fp, err_buf, ERR_BUF_LEN);
        fclose(fp);
        if (qrels == NULL) {
            fprintf(stderr, "Error loading qrels file '%s': %s\n",
              c->qrels_fname, err_buf);
            return NULL;
        }
    }

    num_qids = qrels_get_qids(qrels, qids, MAX_NUM_QIDS);

    /* XXX what we will do is: by default, the qrels file selects
     * the qids that will be assessed.  However, the qids can
     * also be explicitly specified on the command line. 
     * The latter is a TODO.
     *
     * The first thing to do is test whether uncommenting the 
     * following line affects results. */
    for (q = 0; q < num_qids; q++) {
        unsigned id;
        /* pre-assigned all qid ids here, so we get a count of the number
         * of queries. */
        id = strid_get_id(c->qidid, (char *) qids[q]);
    }

    runerr = new_runerr(c->persist, c->qidid, c->max_depth);

    runerr_set_max_num_judgments(runerr, c->max_num_judgments);

    if (qrels != NULL) {
        runerr_set_qrels(runerr, qrels);
    }

    runerr_set_rel_if_unjudged(runerr, c->rel_if_unjudged);

    if (c->score_log_fp != NULL) {
        runerr_log_scores(runerr, c->score_log_fp, c->score_log_interval);
    }

    if (c->judgment_log_fp != NULL) {
        runerr_log_judgments(runerr, c->judgment_log_fp);
    }

    if (c->lacking_log_fp != NULL) {
        runerr_log_lacking_documents(runerr, c->lacking_log_fp);
    }

    if (c->chart_dir) {
        if (runerr_chart(runerr, c->chart_dir, DEFAULT_CHART_DEPTH,
              DEFAULT_CHART_JUDGMENTS) < 0) {
            fprintf(stderr, "Unable to chart to directory '%s': %s\n",
              c->chart_dir, strerror(errno));
            return NULL;
        }
    }

    if (c->signif_log_fp) {
        runerr_log_significance(runerr, c->signif_log_fp,
          c->signif_fn, NULL /* no data for signif function */,
          c->signif_mode, DEFAULT_SIGNIFICANCE_P_THRESHOLD,
          c->signif_log_interval, c->signif_proportion);
    }

    if (runerr_load_runs_from_fnames(runerr, run_fnames,
      num_run_fnames, err_buf, ERR_BUF_LEN) < 0) {
        fprintf(stderr, "Error loading runs: %s\n", err_buf);
        return NULL;
    }

    for (r = 0; r < c->non_contrib_runs.elem_count; r++) {
        const char * runid = c->non_contrib_runs.elems[r];
        if (!runerr_set_run_noncontributing(runerr, runid)) {
            fprintf(stderr, "Unknown run '%s' set non-contributing\n", runid);
            return NULL;
        }
    }

    if (c->stop_when_top_run_found) 
        runerr_stop_when_top_run_found(runerr);
    return runerr;
}

int load_report_depths(char * fname, uint_arr_t * depths) {
    FILE * fp;
    char buf[LINE_BUF_SIZE + 1];
    int last_depth = -1;
    fp = fopen(fname, "r");
    if (fp == NULL) {
        fprintf(stderr, "Unable to open reporting depths "
          "specifier file '%s' for reading\n", fname);
        return -1;
    }
    while ( (fgets(buf, LINE_BUF_SIZE, fp)) != NULL) {
        int depth;
        depth = atoi(buf);
        if (depth <= last_depth) {
            fprintf(stderr, "Report depths must be in strictly increasing "
              "order\n");
            fclose(fp);
            return -1;
        }
        ARRAY_ADD(*depths, (unsigned int) depth);
    }
    return 0;
}

double report_runerr_stats(FILE * fp, runerr_t * runerr, 
  unsigned * prev_num_judged, double * prev_num_relevant) {
    double savg, sdev, smax, precision;
    unsigned max_run_d;
    unsigned new_num_judged;
    double new_num_relevant;
    unsigned num_lacking_judgments;
    savg = runerr_stats(runerr, &sdev, &smax, &max_run_d, &new_num_judged,
      &new_num_relevant, &num_lacking_judgments);
    assert(new_num_relevant >= *prev_num_relevant);
    assert(new_num_judged >= *prev_num_judged);
    precision = (new_num_relevant - *prev_num_relevant) / 
        (new_num_judged - *prev_num_judged);
    fprintf(stdout, "%u %.4lf %.4lf %.4lf %.4lf %d %d\n", new_num_judged, savg, 
      sdev, smax, precision, max_run_d, num_lacking_judgments);
    *prev_num_judged = new_num_judged;
    *prev_num_relevant = new_num_relevant;
    return savg;
}

void common_init(struct common * c) {
    c->persist = DEFAULT_PERSIST;
    c->chart_dir = NULL;
    c->score_log_interval = DEFAULT_SCORE_LOG_INTERVAL;
    c->score_log_fp = NULL;
    c->judgment_log_fp = NULL;
    c->qrels_fname = NULL;
    c->signif_log_fp = NULL;
    c->lacking_log_fp = NULL;
    c->signif_fn = DEFAULT_SIGNIFICANCE_FUNCTION;
    c->signif_log_interval = DEFAULT_SIGNIFICANCE_LOG_INTERVAL;
    c->signif_mode = DEFAULT_SIGNIFICANCE_MODE;
    c->signif_proportion = DEFAULT_SIGNIFICANCE_PROPORTION;
    c->min_avg_err = 0.0;
    c->max_depth = MAX_DEPTH;
    c->qidid = new_strid();
    c->max_num_judgments = DEFAULT_MAX_NUM_JUDGMENTS;
    c->stop_when_top_run_found = 0;
    c->rel_if_unjudged = 0.0;
    ARRAY_INIT(c->non_contrib_runs);
}

int common_process_option(struct common * c, int optflag, char * optarg) {
    switch (optflag) {
    case 'p':
        c->persist = atof(optarg);
        if (c->persist <= 0.0 || c->persist >= 1.0) {
            fprintf(stderr, "Persist (-p) must be in range (0.0, 1.0)\n");
            return -1;
        }
        return 1;
    case 'C':
        c->chart_dir = optarg;
        return 1;
    case 's':
        c->score_log_interval = atoi(optarg);
        if (c->score_log_fp == NULL) {
            c->score_log_fp = stdout;
        }
        return 1;
    case 'S':
        c->score_log_fp = fopen(optarg, "w");
        if (c->score_log_fp == NULL) {
            fprintf(stderr, "Unable to open score log file '%s' for "
              "writing: %s\n", optarg, strerror(errno));
            return -1;
        }
        return 1;
    case 'Q':
        c->qrels_fname = optarg;
        return 1;
    case 'E':
        c->min_avg_err = atof(optarg);
        return 1;
    case 'j':
        c->judgment_log_fp = fopen(optarg, "w");
        if (c->judgment_log_fp == NULL) {
            fprintf(stderr, "Unable to open judgment log file '%s' for "
              "writing: %s\n", optarg, strerror(errno));
            return -1;
        }
        return 1;
    case 'Z':
        c->signif_log_fp = fopen(optarg, "w");
        if (c->signif_log_fp == NULL) {
            fprintf(stderr, "Unable to open significance log file '%s' "
              "for writing: %s\n", optarg, strerror(errno));
            return -1;
        }
        return 1;
    case 'z':
        c->signif_log_interval = atoi(optarg);
        return 1;
    case 'G':
        c->signif_proportion = atof(optarg);
        if (c->signif_proportion < 0.0 || c->signif_proportion > 1.0) {
            fprintf(stderr, "Significance proportion (-G) must be between "
              "0.0 and 1.0\n");
            return -1;
        }
        return 1;
    case 'P':
        if (strcasecmp(optarg, "wilcoxon") == 0) {
            c->signif_fn = paired_wilcoxon_test_p;
        } else if (strcasecmp(optarg, "sign") == 0) {
            c->signif_fn = paired_sign_test_p;
        } else if (strcasecmp(optarg, "t") == 0) {
            c->signif_fn = paired_t_test_p;
        } else if (strcasecmp(optarg, "bootstrap") == 0) {
            c->signif_fn = paired_bootstrap_test_p;
        } else {
            fprintf(stderr, "Unknown significance function '%s'\n", optarg);
            return -1;
        }
        return 1;
    case 'm':
        if (strcasecmp(optarg, "base") == 0) {
            c->signif_mode = SIG_MODE_BASE;
        } else if (strcasecmp(optarg, "conservative") == 0) {
            c->signif_mode = SIG_MODE_CONSERVATIVE;
        } else if (strcasecmp(optarg, "pessimal") == 0) {
            c->signif_mode = SIG_MODE_PESSIMAL;
        } else if (strcasecmp(optarg, "projected") == 0) {
            c->signif_mode = SIG_MODE_PROJECTED;
        } else {
            fprintf(stderr, "Unknown significance mode '%s'\n", optarg);
            return -1;
        }
        return 1;
    case 'J':
        c->max_num_judgments = atoi(optarg);
        return 1;
    case 'N':
        ARRAY_ADD(c->non_contrib_runs, optarg);
        return 1;
    case 'T':
        c->stop_when_top_run_found = 1;
        return 1;
    case 'L':
        c->lacking_log_fp = fopen(optarg, "w");
        if (c->lacking_log_fp == NULL) {
            fprintf(stderr, "Unable to open lacking log file '%s' "
              "for writing: %s\n", optarg, strerror(errno));
            return -1;
        }
        return 1;
    case 'U':
        c->rel_if_unjudged = atof(optarg);
        return 1;
    }
    return 0;
}

#define FCLOSE(fp) if (fp) { fclose(fp); }

void common_cleanup(struct common * c) {
    FCLOSE(c->score_log_fp);
    FCLOSE(c->judgment_log_fp);
    FCLOSE(c->signif_log_fp);
    ARRAY_DELETE(c->non_contrib_runs);
    if (c->qidid)
        strid_delete(&c->qidid);
}
