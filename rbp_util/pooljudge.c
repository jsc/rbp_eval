/*
 *  Choose document to judge using the standard pooling technique.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <getopt.h>
#include "runerr.h"
#include "strhash.h"
#include "dqhash.h"
#include "run.h"
#include "qdocs.h"
#include "qrels.h"
#include "common.h"

#include "wilcoxon.h"
#include "sign.h"
#include "t.h"
#include "bootstrap.h"

#define DEFAULT_POOL_DEPTH 1000
#define DEFAULT_ERR_LOG_INTERVAL 1000

#define USAGE "USAGE: %s <run>...\n"

int main(int argc, char ** argv) {
    unsigned r;
    unsigned d;
    unsigned num_qids;
    runerr_t * runerr = NULL;
    unsigned pool_depth;
    runarr_t * runarr;
    strhash_t * judged_hash;
    int optflag;
    int error = 0;
    unsigned prev_num_judged = 0;
    double prev_num_relevant = 0.0;
    double avg_err = 1.0;
    struct common c;
    int finished;
    int report_at_increment = 1;

    pool_depth = DEFAULT_POOL_DEPTH;

    common_init(&c);

    while ( (optflag = getopt(argc, argv, "d:I" COMMON_OPTS)) != -1) {
        int cret;

        cret = common_process_option(&c, optflag, optarg);
        if (cret == -1) {
            error = 1;
        } else if (cret == 0) {
            switch (optflag) {
            case 'd':
                pool_depth = atoi(optarg);
                break;
            case 'I':
                report_at_increment = 0;
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

    judged_hash = new_strhash();

    runerr = init_runerr(&c, argv + optind, argc - optind);
    if (runerr == NULL) {
        fprintf(stderr, "Error initialising runerr\n");
        return 1;
    }

    num_qids = strid_num_ids(c.qidid);

    runarr = runerr_get_runs(runerr);
    assert(runarr->elems != NULL);
    for (finished = 0, d = 0; !finished && d < pool_depth 
      && (c.min_avg_err == 0.0 || avg_err >= c.min_avg_err); d++) {

        for (r = 0; !finished && r < runarr->elem_count; r++) {
            struct runinfo * ri = &runarr->elems[r];
            run_t * run = ri->run;
            unsigned num_qdocs = run_num_qdocs(run);
            unsigned q;

            if (!ri->contributes_judgments)
                continue;
            for (q = 0; !finished && q < num_qdocs; q++) {
                qdocs_t * qd = run_get_qdocs_by_index(run, q);
                unsigned numranks = qdocs_num_scores(qd);
                doc_score_t * ds = qdocs_get_scores(qd, QDOCS_DEFAULT_ORDERING);
                const char * docid;
                char * qid;
                unsigned qidd;
                int found;

                if (d >= numranks) {
                    continue;
                }
                docid = ds[d].docid;
                qid = qdocs_qid(qd);
                qidd = strid_lookup_id(c.qidid, qid);
                if (qidd == UINT_MAX) {
                    continue;
                }
                dqhash_update(judged_hash, docid, qidd, &found);
                if (!found) {
                    int ret;
                    ret = runerr_doc_judged(runerr, docid, qidd, num_qids);
                    /* FIXME rbp error logging should be part of
                     * common.c or runerr.c */
                    /* XXX in fact for pooljudging you really do want to
                     * do this at each increment rather than at each 1000,
                     * otherwise the max graph looks very jerky. */
                    if (!report_at_increment) {
                        if (runerr_num_judged(runerr) 
                          % DEFAULT_ERR_LOG_INTERVAL == 0) {
                            avg_err = report_runerr_stats(stdout, runerr, 
                              &prev_num_judged, &prev_num_relevant);
                        } 
                    }
                    if (ret == 0)
                        finished = 1;
                }
            }
        }
        if (report_at_increment) {
            avg_err = report_runerr_stats(stdout, runerr, &prev_num_judged,
              &prev_num_relevant);
        }
    }

    strhash_delete(&judged_hash, NULL);

#ifdef CHECK
    /* check that everything has been judged. */
    {
        unsigned r;
        runarr_t * runarr;

        runarr = runerr_get_runs(runerr);
        for (r = 0; r < runarr->elem_count; r++) {
            run_t * run = runarr->elems[r].run;
            unsigned num_qdocs = 0;
            unsigned q;

            num_qdocs = run_num_qdocs(run);
            for (q = 0; q < num_qdocs; q++) {
                unsigned d;
                struct doc_score * ds;
                unsigned num_docs;
                qdocs_t * qd = run_get_qdocs_by_index(run, q);
                char * qid = qdocs_qid(qd);
                unsigned qidd = strid_get_id(c.qidid, qid);

                num_docs = qdocs_num_scores(qd);
                ds = qdocs_get_scores(qd, QDOCS_DEFAULT_ORDERING);
                for (d = 0; d < num_docs; d++) {
                    int found;
                    dqhash_get(judged_hash, ds[d].docid, qidd, &found);
                    assert(found);
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
