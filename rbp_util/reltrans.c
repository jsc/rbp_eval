/*
 * Translate run files into relevance listings.
 *
 * Usage: reltrans [-d <output-dir>] <qrels> <run> ...
 *
 * For each run, creates a subdirectory under <output-dir>
 * with the id of that run.  Then, for each qid in the
 * run, creates a file within subdirectory, with the
 * qid's name.  The contents of the qid file is a list
 * of relevances, one per line, of the ranked documents
 * returned by the run for that qid.
 */

#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "qrels.h"
#include "qdocs.h"
#include "run.h"
#include "futil.h"

#define USAGE "%s [-d <output-dir>] <qrels> <run> ...\n"

#define ERR_BUF_LEN 1024

static int trans_query(FILE * fp, qdocs_t * qdocs, qid_qrels_t * qid_qrels) {
    unsigned num_scores;
    unsigned s;
    doc_score_t * scores;

    num_scores = qdocs_num_scores(qdocs);
    scores = qdocs_get_scores(qdocs, QDOCS_DEFAULT_ORDERING);
    for (s = 0; s < num_scores; s++) {
        rel_t rel;
        rel = qid_qrels_get_rel(qid_qrels, scores[s].docid);
        if (rel < 0.0) {
            fprintf(fp, "NA\n");
        } else {
            fprintf(fp, "%.2lf\n", rel);
        }
    }
    return 0;
}

static int trans_run(char * output_dir, qrels_t * qrels, run_t * run) {
    char sdir_buf[PATH_MAX];
    char fname_buf[PATH_MAX];
    unsigned num_qdocs;
    unsigned q;

    snprintf(sdir_buf, PATH_MAX, "%s/%s", output_dir, run_get_runid(run));
    if (mkdir(sdir_buf, 0777) < 0) {
        fprintf(stderr, "Unable to create run output dir '%s': %s\n",
          sdir_buf, strerror(errno));
        return -1;
    }
    num_qdocs = run_num_qdocs(run);
    for (q = 0; q < num_qdocs; q++) {
        qdocs_t * qdocs;
        qid_qrels_t * qid_qrels;
        char * qid;
        FILE * fp;
        int trans_ret;
        
        qdocs = run_get_qdocs_by_index(run, q);
        qid = qdocs_qid(qdocs);
        qid_qrels = qrels_get_qid_qrels(qrels, qid);
        if (qid_qrels == NULL) {
            warning("unjudged qid '%s' in run '%s'", qid, run_get_runid(run));
            continue;
        }
        snprintf(fname_buf, PATH_MAX, "%s/%s", sdir_buf, qid);
        fp = fopen(fname_buf, "w");
        if (!fp) {
            fprintf(stderr, "Unable to open output file '%s' for writing: %s\n",
              fname_buf, strerror(errno));
            return -1;
        }
        trans_ret = trans_query(fp, qdocs, qid_qrels);
        fclose(fp);
        if (trans_ret < 0) {
            return -1;
        }
    }
    return 0;
}

static int do_trans(char * output_dir, char * qrels_fname,
  char ** run_fnames, unsigned num_runs) {
    FILE * qrels_fp = NULL;
    qrels_t * qrels = NULL;
    int ret;
    char err_buf[ERR_BUF_LEN];
    unsigned r;

    qrels_fp = fopen(qrels_fname, "r");
    if (qrels_fp == NULL) {
        fprintf(stderr, "Unable to open qrels file '%s' for reading: %s\n",
          qrels_fname, strerror(errno));
        goto ERROR;
    }
    qrels = load_qrels(qrels_fp, err_buf, ERR_BUF_LEN);
    fclose(qrels_fp);
    if (qrels == NULL) {
        fprintf(stderr, "Error loading qrels file '%s': %s\n",
          qrels_fname, strerror(errno));
        goto ERROR;
    }

    for (r = 0; r < num_runs; r++) {
        FILE * run_fp = NULL;
        run_t * run = NULL;
        int trans_ret;

        run_fp = fopen(run_fnames[r], "r");
        if (run_fp == NULL) {
            fprintf(stderr, "Unable to open run file '%s' for reading: %s\n",
              run_fnames[r], strerror(errno));
            goto ERROR;
        }
        run = load_run(run_fp, err_buf, ERR_BUF_LEN);
        fclose(run_fp);
        if (run == NULL) {
            fprintf(stderr, "Error loading run file '%s': %s\n",
              run_fnames[r], err_buf);
            goto ERROR;
        }
        trans_ret = trans_run(output_dir, qrels, run);

        run_delete(&run);
        if (trans_ret < 0) {
            goto ERROR;
        }
    }

    ret = 0;
    goto END;

ERROR:
    ret = -1;

END:
    qrels_delete(&qrels);
    return ret;

}

int main(int argc, char ** argv) {
    int optflag;
    char * output_dir = ".";
    char ** run_files;
    unsigned num_runs;
    char * qrels_file;
    int error = 0;

    while ( (optflag = getopt(argc, argv, "d:")) != -1) {
        switch (optflag) {
        case 'd':
            output_dir = optarg;
            break;
        case '?':
            fprintf(stderr, "Unknown option '%c'\n", optflag);
            error = 1;
            break;
        }
    }

    if (!futil_is_writeable_dir(output_dir)) {
        fprintf(stderr, "%s not a writeable valid directory: %s\n",
          output_dir, strerror(errno));
        error = 1;
    }

    if (argc - optind < 2) {
        error = 1;
    } else {
        unsigned r;
        qrels_file = argv[optind];
        run_files = argv + optind + 1;
        num_runs = argc - optind - 1;
        if (!futil_is_readable_file(qrels_file)) {
            fprintf(stderr, "Qrels file %s is not readable: %s\n",
              qrels_file, strerror(errno));
        }
        for (r = 0; r < num_runs; r++) {
            if (!futil_is_readable_file(run_files[r])) {
                fprintf(stderr, "Run file %s is not readable: %s\n",
                  run_files[r], strerror(errno));
            }
        }
    }

    if (error) {
        fprintf(stderr, USAGE, argv[0]);
        exit(1);
    }

    do_trans(output_dir, qrels_file, run_files, num_runs);

    return 0;
}
