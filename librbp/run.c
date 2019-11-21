#include "config.h"
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <ctype.h>

#include "error.h"
#include "util.h"
#include "run.h"
#include "strhash.h"

/* causes too many compatibility problems... */
#undef RUN_MD5SUM

#ifdef RUN_MD5SUM
#include <openssl/md5.h>
#endif /* RUN_MD5SUM */

#define LINE_BUF_SZ 1024
#define QID_BUF_SZ 256
#define RUNID_BUF_SZ 256

#define QDOCS_INIT_SZ 1024
#define QDOCS_EXP_FACTOR 2.0

#define RUN_NUM_COLS 6
#define RUN_QID_COL 0
#define RUN_ITER_COL 1
#define RUN_DOCID_COL 2
#define RUN_RANK_COL 3
#define RUN_SCORE_COL 4
#define RUN_RUNID_COL 5

struct run {
    qdocs_t ** qdocs;
    unsigned qdocs_num;
    unsigned qdocs_size;
    strhash_t * qdocs_hash;
    unsigned max_depth;
    char * runid;
#ifdef RUN_MD5SUM
#define MD5_HEX_LENGTH 32
    char md5sum[MD5_HEX_LENGTH + 1];
#endif /* RUN_MD5SUM */
};

static run_t * _new_run();
static qdocs_t * _get_create_qdocs(run_t * run, char * qid, int create);

run_t * load_run(FILE * fp, char * err_buf, unsigned err_buf_len) {
    return load_run_single_query(fp, NULL, err_buf, err_buf_len);
}

run_t * load_run_single_query(FILE * fp, char * qid, char * err_buf, 
  unsigned err_buf_len) {

    char line_buf[LINE_BUF_SZ];
    char qid_buf[QID_BUF_SZ];
    char runid_buf[RUNID_BUF_SZ];
    int warned_about_runid = 0;
    unsigned line_num = 0;
    qdocs_t * qd = NULL;
    run_t * run;
    unsigned q;
#ifdef RUN_MD5SUM
    MD5_CTX md5_ctx;
    unsigned char md5_digest[MD5_DIGEST_LENGTH];
    int c;
#endif /* RUN_MD5SUM */

    qid_buf[0] = '\0';
    runid_buf[0] = '\0';
    run = _new_run();
#ifdef RUN_MD5SUM
    MD5_Init(&md5_ctx);
#endif /* RUN_MD5SUM */
    while (fgets(line_buf, LINE_BUF_SZ, fp) != NULL) {
        char * cols[RUN_NUM_COLS];
        int ret;
        double score;
        long rank;
        char * score_end;
        char * rank_end;

#ifdef RUN_MD5SUM
        MD5_Update(&md5_ctx, line_buf, strlen(line_buf));
#endif /* RUN_MD5SUM */
        line_num++;
        ret = util_parse_cols(line_buf, cols, RUN_NUM_COLS);
        if (ret < 0) {
            if (err_buf)
                snprintf(err_buf, err_buf_len,
                  "wrong number of fields on line %d of run file", line_num);
            run_delete(&run);
            return NULL;
        }
        util_downcase_str(cols[RUN_QID_COL]);
        if (qid != NULL && strcmp(qid, cols[RUN_QID_COL]) != 0) {
            continue;
        }
        if (strcmp(cols[RUN_QID_COL], qid_buf) != 0) {
            strncpy(qid_buf, cols[RUN_QID_COL], QID_BUF_SZ);
            qid_buf[QID_BUF_SZ - 1] = '\0';
            qd = _get_create_qdocs(run, cols[RUN_QID_COL], 1);
        }
        score = strtod(cols[RUN_SCORE_COL], &score_end);
        if (*score_end != '\0') {
            if (err_buf)
                snprintf(err_buf, err_buf_len,
                  "score '%s' not a floating point number on line "
                  "%d of run file", cols[RUN_SCORE_COL], line_num);
            run_delete(&run);
            return NULL;
        } else if (isnan(score)) {
            /* The odd run puts in a single document with a score
             * of NaN as a placeholder for a query that it otherwise
             * would not have any results for. */
            warning("score for qid %s, docid %s, on line "
              "%d of run file is NaN, converting to 0.0\n",  cols[RUN_QID_COL],
              cols[RUN_DOCID_COL], line_num);
            score = 0.0;
        }
        errno = 0;
        /* strtoul does automatic negative-to-positive conversion, which
         * is not what we want... */
        rank = strtol(cols[RUN_RANK_COL], &rank_end, 10);
        if (*rank_end == ',') {
            /* incredibly, some runs format ranks with commas */
            static int warned_about_commas = 0;
            char * f, * t;

            if (!warned_about_commas) {
                warning("comma(s) in rank on line %d of run "
                  "file, stripping", line_num);
                warned_about_commas = 1;
            }
            for (f = t = rank_end; ; f++, t++) {
                while (*f == ',')
                    f++;
                *t = *f;
                if (*f == '\0')
                    break;
            }
            rank = strtol(cols[RUN_RANK_COL], &rank_end, 10);
        }
        if (errno == ERANGE || rank < 0 || *rank_end != '\0') {
            if (err_buf)
                snprintf(err_buf, err_buf_len,
                  "rank not a non-negative integer on line %d of run file",
                  line_num);
            run_delete(&run);
            return NULL;
        }
        if (runid_buf[0] != '\0' && strcmp(cols[RUN_RUNID_COL], runid_buf)
          != 0 && !warned_about_runid) {
            warning("runid changes on line %d of run file: "
              "was '%s', is now '%s'", line_num, runid_buf, 
              cols[RUN_RUNID_COL]);
            warned_about_runid = 1;
        } else if (runid_buf[0] == '\0') {
            strncpy(runid_buf, cols[RUN_RUNID_COL], RUNID_BUF_SZ);
        }
        qdocs_add_doc_score(qd, cols[RUN_DOCID_COL], (unsigned) rank, score);
    }
#ifdef RUN_MD5SUM
    MD5_Final(md5_digest, &md5_ctx);
    for (c = 0; c < MD5_DIGEST_LENGTH; c++) {
        sprintf(run->md5sum + c * 2, "%02x", md5_digest[c]);
    }
#endif /* RUN_MD5SUM */
    for (q = 0; q < run->qdocs_num; q++) {
        unsigned depth = qdocs_num_scores(run->qdocs[q]);
        if (depth > run->max_depth)
            run->max_depth = depth;
    }
    run->runid = util_strdup_or_die(runid_buf);
    return run;
}

#ifdef RUN_MD5SUM
char * run_get_md5sum(run_t * run) {
    return run->md5sum;
}
#else
char * run_get_md5sum(run_t * run) {
    return "";
}
#endif /* RUN_MD5SUM */

const char * run_get_runid(run_t * run) {
    return run->runid;
}

void run_delete(run_t ** run_p) {
    unsigned i;
    run_t * run = *run_p;
    strhash_delete(&run->qdocs_hash, NULL);
    for (i = 0; i < run->qdocs_num; i++) {
        qdocs_t * qd = run->qdocs[i];
        qdocs_delete(&qd);
    }
    free(run->qdocs);
    free(run->runid);
    free(run);
    *run_p = NULL;
}

unsigned run_num_qdocs(run_t * run) {
    return run->qdocs_num;
}

qdocs_t * run_get_qdocs_by_index(run_t * run, unsigned index) {
    assert(index < run->qdocs_num);
    return run->qdocs[index];
}

qdocs_t * run_get_qdocs_by_qid(run_t * run, char * qid) {
    return _get_create_qdocs(run, qid, 0);
}

static run_t * _new_run() {
    run_t * run;
    run = util_malloc_or_die(sizeof(*run));
    run->qdocs = NULL;
    run->qdocs_num = 0;
    run->qdocs_size = 0;
    run->qdocs_hash = new_strhash();
    run->max_depth = 0;
#ifdef RUN_MD5SUM
    run->md5sum[0] = '\0';
#endif /* RUN_MD5SUM */
    run->runid = NULL;
    return run;
}

static qdocs_t * _get_create_qdocs(run_t * run, char * qid, int create) {
    qdocs_t ** qd_p;

    qd_p = (qdocs_t **) strhash_update(run->qdocs_hash, qid, NULL);
    if (*qd_p == NULL && create) {
        util_ensure_array_space((void **) &run->qdocs, &run->qdocs_size,
          run->qdocs_num, sizeof(*run->qdocs), QDOCS_INIT_SZ,
          QDOCS_EXP_FACTOR);
        *qd_p = new_qdocs(qid);
        run->qdocs[run->qdocs_num++] = *qd_p;
    }
    return *qd_p;
}

#ifdef RUN_MAIN

#define ERR_BUF_SIZE 1024

int main(int argc, char ** argv) {
    char * fname;
    FILE * fp;
    run_t * run;
    char err_buf[ERR_BUF_SIZE];

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
    run = load_run(fp, err_buf, ERR_BUF_SIZE);
    if (run == NULL) {
        fprintf(stderr, "Error loading run file '%s': %s\n", fname,
          err_buf);
        fclose(fp);
        return -1;
    }
    run_delete(&run);
    fclose(fp);
    return 0;
}

#endif /* RUN_MAIN */
