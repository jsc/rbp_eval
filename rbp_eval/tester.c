#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "rbp.h"
#include "qrels.h"
#include "res.h"
#include "util.h"
#include "opt.h"

#define SPEC_BUF_SIZE 1024

#define MARGIN_OF_ERROR 0.0001

int getline_or_die(char * buf, int size, FILE * fp) {
    char * ret;
    int lines = 0;
    int len;
    errno = 0;
    do {
        ret = fgets(buf, size, fp);
        if (ret == NULL) {
            if (errno != 0) {
                perror("reading from file");
            } else {
                fprintf(stderr, "premature end to spec file\n");
            }
            exit(1);
        }
        lines++;
    } while (buf[0] == '#');
    len = strlen(buf);
    if (buf[len - 1] != '\n') {
        fprintf(stderr, "Line too long\n");
        exit(1);
    }
    buf[len - 1] = '\0';
    return lines;
}

FILE * fopen_or_die(const char * fname) {
    FILE * fp;
    fp = fopen(fname, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error opening file '%s' for reading: %s\n",
          fname, strerror(errno));
        exit(1);
    }
    return fp;
}

#define MAX_OPTS 256

int parse_args(char * line, char ** args, unsigned max_args) {
    char * pos;
    int o;

    args[0] = "tester";
    for (o = 1, pos = line; o < max_args && *pos != '\0'; o++) {
        args[o] = pos;
        pos = util_delim_word(pos);
        pos = util_next_nonspace(pos);
    }
    args[o] = NULL;
    return o;
}

int main(int argc, char ** argv) {
    FILE * spec_fp;
    char spec_buf[SPEC_BUF_SIZE];
    qrels_t * qrels = NULL;
    run_t * run = NULL;
    FILE * fp;
    depth_t depth;
    persist_t persist;
    enum qdocs_ord_t ord;
    res_t * res = NULL;
    unsigned q, d, p;
    unsigned linenum;
    char * args[MAX_OPTS];
    unsigned num_args;
    struct opt opt;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <test-spec>\n", argv[0]);
        return -1;
    }
    spec_fp = fopen_or_die(argv[1]);

    linenum = 0;
#define GETLINE (linenum += getline_or_die(spec_buf, SPEC_BUF_SIZE, spec_fp))
#define CHECK_FLOAT_LINE(val) {                 \
    double vv = val, sv;                        \
    GETLINE;                                    \
    if (sscanf(spec_buf, "%lf", &sv) != 1) {    \
        fprintf(stderr, "Invalid input '%s' on line %u\n",  \
          spec_buf, linenum);                   \
        exit(1);                                \
    }                                           \
    if (fabs(vv - sv) > MARGIN_OF_ERROR) {      \
        fprintf(stderr, "Incorrect value, is %lf, should be %lf, " \
          "on line %u\n", vv, sv, linenum);     \
        exit(1);                                \
    }                                           \
}
    GETLINE;
    num_args = parse_args(spec_buf, args, MAX_OPTS);
    assert(args[num_args] == NULL);
    if (opt_process(&opt, num_args, args) < 0) {
        exit(1);
    }

    /* qrels file */
    fp = fopen_or_die(opt.qrels_fname);
    qrels = load_qrels(fp, NULL, 0);
    if (qrels == NULL) {
        fprintf(stderr, "Error loading qrels file '%s'\n", spec_buf);
        exit(1);
    }
    fclose(fp);

    /* run file */
    fp = fopen_or_die(opt.run_fname);
    run = load_run(fp, NULL, 0);
    if (run == NULL) {
        fprintf(stderr, "Error loading run file '%s'\n", spec_buf);
        exit(1);
    }
    fclose(fp);

    /* depth spec */
    memcpy(&depth, &opt.depth, sizeof(depth));

    /* p spec */
    memcpy(&persist, &opt.persist, sizeof(persist));

    /* ord type */
    ord = opt.ord;
    
    qrels_set_reltype(qrels, opt.reltype, opt.reltype_arg);

    /* now, perform rbp calculations and check against values in file. */
    res = evaluate_res(qrels, run, ord, &persist, &depth);

    for (q = 0; q <= res->num_qid; q++) {
        qid_res_t * qres;
        if (q < res->num_qid) {
            qres = &res->qid_res[q];
        } else {
            qres = &res->ave_res;
        }
        CHECK_FLOAT_LINE(qres->num_rel);
        CHECK_FLOAT_LINE(qres->num_ret);
        for (d = 0; d < depth.d_num; d++) {
            CHECK_FLOAT_LINE(qres->depth_res[d].num_rel_ret);
            for (p = 0; p < persist.p_num; p++) {
                double sum, err;
                persist_res_t * pres;
                GETLINE;
                if (sscanf(spec_buf, "%lf %lf", &sum, &err) != 2) {
                    fprintf(stderr, "Invalid input '%s' on line %u\n", 
                      spec_buf, linenum);
                    exit(1);
                }
                pres = &qres->depth_res[d].persist_res[p];
                if (fabs(sum - pres->sum) > MARGIN_OF_ERROR) {
                    fprintf(stderr, "Incorrect sum, is %lf, should be %lf, "
                      "on line %u\n", pres->sum, sum, linenum);
                    exit(1);
                }
                if (fabs(err - pres->err) > MARGIN_OF_ERROR) {
                    fprintf(stderr, "Incorrect err, is %lf, should be %lf, "
                      "on line %u\n", pres->err, err, linenum);
                    exit(1);
                }
            }
        }
    }

#undef GETLINE
#undef CHECK_FLOAT_LINE
    res_delete(&res);
    qrels_delete(&qrels);
    run_delete(&run);
    fclose(spec_fp);
    return 0;
}
