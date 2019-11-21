/*
 *  Chose documents to judge so as to minimise average error.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <math.h>
#include <unistd.h>
#include "run.h"
#include "strid.h"
#include "util.h"
#include "docwgt.h"
#include "dococcur.h"
#include "rbp.h"
#include "runerr.h"
#include "array.h"
#include "common.h"
#include "qrels.h"
#include "dblheap.h"

#define DEFAULT_ERR_LOG_INTERVAL 1000

#define USAGE "USAGE: %s <run>...\n"

/* document choices recalculated at 
 * num_qids * WEIGHTED_RECALC_INTERVAL_MULT */
#define WEIGHTED_RECALC_INTERVAL_MULT 5

int main(int argc, char ** argv) {
    unsigned r;
    unsigned d;
    docwgt_t * docwgt = NULL;
    unsigned num_qids;
    unsigned num_ranked_docs;
    docwgt_elem_t * docwgt_docs;
    runerr_t * runerr = NULL;
    int optflag;
    int error = 0;
    uint_arr_t report_depths;
    unsigned rd;
    runarr_t * runarr;
    enum weight_t weighting = WGT_UNIFORM;
    enum query_weight_t qry_weighting = WGT_QRY_UNIFORM;
    double * run_wgts;
    double * qry_wgts;
    double rbp_wgts[MAX_DEPTH];
    dblheap_t * wgt_heap;
    unsigned * wgt_list;
    unsigned wgt_ind = 0;
    unsigned weighted_recalc_interval;
    unsigned prev_num_judged = 0;
    double prev_num_relevant = 0.0;
    double avg_err = 1.0;
    struct common c;
    int finished;

    ARRAY_INIT(report_depths);

    common_init(&c);

    while ( (optflag = getopt(argc, argv, "D:wW:q:" COMMON_OPTS)) != -1) {
        int cret;

        cret = common_process_option(&c, optflag, optarg);
        if (cret == -1) {
            error = 1;
        } else if (cret == 0) {
            switch (optflag) {
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
                } else if (strcmp(optarg, "projected") == 0) {
                    weighting = WGT_PROJECTED;
                } else if (strcmp(optarg, "residual") == 0) {
                    weighting = WGT_RESIDUAL;
                } else if (strcmp(optarg, "residual-and-midpoint") == 0) {
                    weighting = WGT_RESIDUAL_AND_MIDPOINT;
                } else if (strcmp(optarg, 
                      "residual-and-midpoint-squared") == 0) {
                    weighting = WGT_RESIDUAL_AND_MIDPOINT_SQ;
                } else if (strcmp(optarg, 
                      "residual-and-midpoint-cubed") == 0) {
                    weighting = WGT_RESIDUAL_AND_MIDPOINT_CB;
                } else if (strcmp(optarg, "residual-and-projected") == 0) {
                    weighting = WGT_RESIDUAL_AND_PROJECTED;
                } else if (strcmp(optarg, "residual-and-projected-squared") == 0) {
                    weighting = WGT_RESIDUAL_AND_PROJECTED_SQ;
                } else if (strcmp(optarg, "midpoint-squared") == 0) {
                    weighting = WGT_MIDPOINT_SQ;
                } else {
                    fprintf(stderr, "Unknown weighting strategy '%s'\n",
                      optarg);
                    error = 1;
                }
                break;
            case 'q':
                if (strcmp(optarg, "uniform") == 0) {
                    qry_weighting = WGT_QRY_UNIFORM;
                } else if (strcmp(optarg, "linear") == 0) {
                    qry_weighting = WGT_QRY_LINEAR;
                } else {
                    fprintf(stderr, "Unknown weighting strategy '%s'\n",
                      optarg);
                    error = 1;
                }
                break;
            case '?':
                fprintf(stderr, "Unknown option '%c'\n", optflag);
                error = 1;
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

    docwgt = new_docwgt(c.persist);

    runerr = init_runerr(&c, argv + optind, argc - optind);
    if (runerr == NULL) {
        fprintf(stderr, "Error initialising runerr\n");
        return 1;
    }

    runarr = runerr_get_runs(runerr);
    for (r = 0; r < runarr->elem_count; r++) {
        struct runinfo * ri = &runarr->elems[r];
        run_t * run = ri->run;
        if (ri->contributes_judgments)
            docwgt_add_run(docwgt, run, c.qidid);
    }

    num_qids = strid_num_ids(c.qidid);
    num_ranked_docs = docwgt_num_entries(docwgt);

    if (weighting != WGT_UNIFORM || qry_weighting != WGT_QRY_UNIFORM) {
        weighted_recalc_interval = WEIGHTED_RECALC_INTERVAL_MULT * num_qids;
        if (weighted_recalc_interval < 2)
            weighted_recalc_interval = 2;
        wgt_heap = new_dblheap(DBLHEAP_MIN);
        wgt_list = util_malloc_or_die(sizeof(*wgt_list) * weighted_recalc_interval);
    }

    docwgt_docs = util_malloc_or_die(sizeof(*docwgt_docs) * num_ranked_docs);
    docwgt_get_entries(docwgt, docwgt_docs, num_ranked_docs, 1);

    run_wgts = util_malloc_or_die(sizeof(*run_wgts) * runarr->elem_count);
    qry_wgts = util_malloc_or_die(sizeof(*qry_wgts) * num_qids);

    rbp_weights(rbp_wgts, c.persist, MAX_DEPTH);

    for (finished = 0, rd = 0, d = 0; !finished && d < num_ranked_docs 
      && (report_depths.elem_count == 0 || rd < report_depths.elem_count) 
      && (c.min_avg_err == 0.0 || avg_err >= c.min_avg_err); d++) {
        const char * docid;
        unsigned qidd;
        unsigned judge_next;
        /* unsigned num_judged;
        double num_relevant; */
        docwgt_elem_t * dw;

        if ((report_depths.elem_count == 0 
              && d % DEFAULT_ERR_LOG_INTERVAL == 0) || 
          (report_depths.elem_count != 0 && report_depths.elems[rd] == d)) {
            avg_err = report_runerr_stats(stdout, runerr, &prev_num_judged,
              &prev_num_relevant);
            rd++;
        }
        if (weighting != WGT_UNIFORM || qry_weighting != WGT_QRY_UNIFORM) {
            double ignore;
            unsigned heap_size = dblheap_size(wgt_heap);

            if (wgt_ind == 0) {
                double max_prop_err;
                double threshold = -1.0;
                unsigned max_run_d, d2, r, q;
                unsigned considered = 0;

                /* Calculate the maximum proportional weight across the
                 * runs.  This times the rbp weight of a document sets
                 * the upper bound for a document's weighted value
                 * (i.e., gives the weighted value if all of the document's
                 * rbp weight were in the highest-weighted run).  Since
                 * the documents are ordered by rbp weight, this allows
                 * us to terminate the inner loop once the highest
                 * weight found so far exceeds the upper bound weight
                 * of the next document. */
                max_prop_err = runerr_max_prop_wgt(runerr, &max_run_d,
                  weighting);
                for (r = 0; r < runarr->elem_count; r++)
                    run_wgts[r] = -1.0;
                for (q = 0; q < num_qids; q++) 
                    qry_wgts[q] = -1.0;

                for (d2 = 0; d2 < num_ranked_docs; d2++) {
                    dococcur_item_array_t * dcrarr;
                    double err = 0.0;
                    unsigned dc;
                    docwgt_elem_t * dw;

                    /* NOTE: in the following calculation, both err_wgt
                     * and dw->wgt are not normalised by the number of
                     * qids.  Since they are compared only with each
                     * other, this doesn't matter, but if you change
                     * one, you need to change the other. */
                    dw = &docwgt_docs[d2];
                    if (dw->flags == 1) {
                        continue;
                    }
                    if (dw->wgt * max_prop_err < threshold) {
                        break;
                    }
                    considered++;
                    dcrarr = runerr_get_dococcur_items(runerr, dw->docid,
                      dw->qidd);
                    for (dc = 0; dc < dcrarr->elem_count; dc++) {
                        dococcur_item_t * di = &dcrarr->elems[dc];
                        double run_wgt, qry_wgt, rbp_wgt, err_wgt;

                        if (runarr->elems[di->rund].contributes_judgments) {
                            if (run_wgts[di->rund] < 0.0) {
                                run_wgts[di->rund] 
                                    = runerr_prop_err_wgt_for_run(runerr, 
                                      di->rund, weighting);
                            }
                            if (qry_wgts[dw->qidd] < 0.0) {
                                qry_wgts[dw->qidd] = runerr_wgt_for_qry(runerr,
                                      dw->qidd, qry_weighting);
                            }
                            run_wgt = run_wgts[di->rund];
                            qry_wgt = qry_wgts[dw->qidd];
                            rbp_wgt = rbp_wgts[di->rank];
                            err_wgt = rbp_wgt * run_wgt * qry_wgt;
                            err += err_wgt;
                        }
                    }
                    if (heap_size >= weighted_recalc_interval) {
                        if (err > threshold) {
                            dblheap_push(wgt_heap, err, (void *) d2);
                            dblheap_pop(wgt_heap, &ignore);
                            dblheap_peek(wgt_heap, &threshold);
                        }
                    } else {
                        dblheap_push(wgt_heap, err, (void *) d2);
                        heap_size++;
                    }
                }
                while (heap_size > 0) {
                    heap_size--;
                    wgt_list[wgt_ind++] 
                        = (unsigned) dblheap_pop(wgt_heap, &ignore);
                }
            }
            assert(wgt_ind > 0);
            judge_next = wgt_list[wgt_ind - 1];
            wgt_ind--;
        } else {
            judge_next = d;
        }
        dw = &docwgt_docs[judge_next];
        docid = dw->docid;
        qidd = dw->qidd;
        /* mark that a document/qid pair has been judged. */
        dw->flags = 1;
        if (!runerr_doc_judged(runerr, docid, qidd, num_qids))
            finished = 1;
    }

    if (runerr) {
        runerr_delete(&runerr);
    }
    if (docwgt) {
        docwgt_delete(&docwgt);
    }
    common_cleanup(&c);

    return 0;
}
