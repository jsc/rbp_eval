/*
 *  Choose documents to judge so as to minimise the maximum error.
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include "strid.h"
#include "common.h"
#include "run.h"
#include "runerr.h"
#include "error.h"

#define ERR_BUF_LEN 1024

#define USAGE "USAGE: %s <run>...\n"

#define MAX_QIDS 1000
#define DEFAULT_DEPTH 1000

int main(int argc, char ** argv) {
    unsigned d;
    int optflag;
    int error = 0;
    uint_arr_t report_depths;
    runarr_t * runarr;
    runerr_t * runerr = NULL;
    unsigned num_qids;
    int reached_depth = 0;
    unsigned depth = DEFAULT_DEPTH;
    unsigned rd;
    unsigned short_run = 0;
    enum weight_t weighting = WGT_UNIFORM;
    unsigned prev_num_judged = 0;
    double prev_num_relevant = 0.0;
    double avg_err = 1.0;
    struct common c;
    int finished;

    ARRAY_INIT(report_depths);

    common_init(&c);

    while ( (optflag = getopt(argc, argv, "D:wW:" COMMON_OPTS)) != -1) {
        int cret;

        cret = common_process_option(&c, optflag, optarg);
        if (cret == -1) {
            error = 1;
        } else if (cret == 0) {
            switch(optflag) {
            case 'D': 
                if (load_report_depths(optarg, &report_depths) < 0) {
                    error = 1;
                }
                break;
            case 'w':
                weighting = WGT_QUADRATIC;
                break;
            case 'W':
                if (strcmp(optarg, "quadratic") == 0) {
                    weighting = WGT_QUADRATIC;
                } else if (strcmp(optarg, "linear") == 0) {
                    weighting = WGT_LINEAR;
                } else if (strcmp(optarg, "uniform") == 0) {
                    weighting = WGT_UNIFORM;
                } else {
                    fprintf(stderr, "Unknown weighting strategy '%s'\n", optarg);
                    error = 1;
                }
                break;
            case '?':
                fprintf(stderr, "Unknown option '%c'\n", optflag);
                break;
            }
        }
    }
    if (error) {
        exit(1);
    }

    if (argc - optind < 1) {
        fprintf(stderr, USAGE, argv[0]);
        exit(1);
    }

    runerr = init_runerr(&c, argv + optind, argc - optind);
    if (runerr == NULL) {
        fprintf(stderr, "Error initialising runerr\n");
        return 1;
    }

    runarr = runerr_get_runs(runerr);

    num_qids = strid_num_ids(c.qidid);

    for (finished = 0, rd = 0, d = 0; !short_run && !reached_depth 
      && (report_depths.elem_count == 0 || rd < report_depths.elem_count)
      && (c.min_avg_err == 0.0 || avg_err >= c.min_avg_err); d++) {
        double savg, sdev, smax;
        unsigned max_run_d;
        struct runinfo * ri;
        unsigned num_qdocs;
        unsigned q;
        unsigned judged_one = 0;
        unsigned num_judged;
        double num_relevant;
        unsigned lacking_judgments;

        short_run = 1;
        savg = runerr_stats_wgt(runerr, &sdev, &smax, &max_run_d, &num_judged,
          &num_relevant, weighting, &lacking_judgments);
        ri = &runarr->elems[max_run_d];
        if ((report_depths.elem_count == 0 && d % 1000 == 0) || 
          (report_depths.elem_count != 0 && report_depths.elems[rd] == d)) {
            avg_err = report_runerr_stats(stdout, runerr, &prev_num_judged,
              &prev_num_relevant);
            rd++;
        }
        num_qdocs = run_num_qdocs(ri->run);
        do {
            /* XXX we possibly don't want to abort here, but keep going,
             * assumed unjudged documents are irrelevant. */
            if (ri->judged_depth == depth) {
                reached_depth = 1;
                fprintf(stderr, "Exhausted judgments for run %s at depth %d"
                  ": %.4lf +%.4lf\n", run_get_runid(ri->run), depth,
                  ri->rbp, ri->err);
                break;
            }
            for (q = 0; !finished && q < num_qdocs && !judged_one; q++) {
                qdocs_t * qd = run_get_qdocs_by_index(ri->run, q);
                unsigned numranks = qdocs_num_scores(qd);
                if (ri->judged_depth < numranks) {
                    doc_score_t * ds;

                    short_run = 0;
                    ds = qdocs_get_scores(qd, QDOCS_DEFAULT_ORDERING);
                    if (ds[ri->judged_depth].flags == 0) {
                        char * qid;
                        unsigned qidd;
                        dococcur_item_array_t * dcr;
                        unsigned i;

                        judged_one = 1;
                        qid = qdocs_qid(qd);
                        qidd = strid_get_id(c.qidid, qid);
                        /* fprintf(stderr, "Judging depth %d, query %d, of run %d, err %lf, rbp %lf\n",
                          ri->judged_depth, q, max_run_d, ri->err, ri->rbp); */
                        if (!runerr_doc_judged(runerr, 
                              ds[ri->judged_depth].docid, qidd, num_qids))
                            finished = 1;
                        dcr = runerr_get_dococcur_items(runerr, 
                          ds[ri->judged_depth].docid, qidd);
                        for (i = 0; i < dcr->elem_count; i++) {
                            dococcur_item_t * di;
                            doc_score_t * ds2;

                            di = &dcr->elems[i];
                            ds2 = di->data;
                            ds2->flags = 1;
                        }
                    }
                }
            }
            if (!judged_one) {
                /* all qids at this depth have been judged; advance to the
                 * next depth. */
                ri->judged_depth++;
            }
        } while (!finished && !reached_depth && !judged_one && !short_run);
        if (short_run) {
            warning("finishing processing as run %s has no more judgments "
              "at depth %u", run_get_runid(ri->run), ri->judged_depth);
        }
    }

#ifdef CHECK
    /* check that everything has been judged. */
    /* NOTE: due to accumulated inaccuracies in the floating point
     * calculation, it can (and generally does) happen that one
     * run continues to have the maximum error even when it has
     * been exhaustively evaluated and other runs have not. 
     * This is why we define a "reasonable" depth that we should
     * expect all runs to have been evaluated to. */
#define REASONABLE_DEPTH ((DEFAULT_DEPTH / 2))
    if (!short_run && report_depths.elem_count == 0) {
        unsigned r;
        runarr_t * runarr;

        runarr = runerr_get_runs(runerr);
        for (r = 0; r < runarr->elem_count; r++) {
            run_t * run = runarr->elems[r].run;
            unsigned num_qdocs = 0;
            unsigned q;

            num_qdocs = run_num_qdocs(run);
            for (q = 0; q < num_qdocs; q++) {
                struct doc_score * ds;
                unsigned num_docs;
                qdocs_t * qd = run_get_qdocs_by_index(run, q);

                num_docs = qdocs_num_scores(qd);
                ds = qdocs_get_scores(qd, QDOCS_DEFAULT_ORDERING);
                for (d = 0; d < num_docs && d < REASONABLE_DEPTH; d++) {
                    assert(ds[d].flags == 1);
                }
            }
        }
    }

#endif /* CHECK */

    if (runerr) {
        runerr_delete(&runerr);
    }
    common_cleanup(&c);

    return 0;
}
