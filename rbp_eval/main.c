#include "rbp.h"
#include "opt.h"
#include "qrels.h"
#include "qdocs.h"
#include "run.h"
#include "error.h"
#include "util.h"
#include "res.h"
#include "fmt.h"
#include "desc_fmt.h"
#include "trec_fmt.h"
#include "help.h"

#define ERR_BUF_LEN 1024

int main(int argc, char ** argv) {
    int ret;
    struct opt opt;
    FILE * qrels_fp = NULL;
    FILE * run_fp = NULL;
    run_t * run = NULL;
    qrels_t * qrels = NULL;
    res_t * res = NULL;
    char err_buf[ERR_BUF_LEN];
    double max_rel;
    fmt_args_t fmt_args;

    ret = opt_process(&opt, argc, argv);
    if (ret != 1)
        return ret;

    if (opt.no_warnings) {
        warning_set_stream(NULL);
    }

    qrels_fp = fopen(opt.qrels_fname, "r");
    if (qrels_fp == NULL) {
        fprintf(stderr, "Unable to open qrels file %s for reading\n",
          opt.qrels_fname);
        print_help(argv[0], stderr);
        goto ERROR;
    }
    run_fp = fopen(opt.run_fname, "r");
    if (run_fp == NULL) {
        fprintf(stderr, "Unable to open run file %s for reading\n",
          opt.run_fname);
        print_help(argv[0], stderr);
        goto ERROR;
    }

    run = load_run(run_fp, err_buf, ERR_BUF_LEN);
    if (run == NULL) {
        fprintf(stderr, "Error loading run file: %s\n", err_buf);
        print_help(argv[0], stderr);
        goto ERROR;
    }

    qrels = load_qrels(qrels_fp, err_buf, ERR_BUF_LEN);
    if (qrels == NULL) {
        fprintf(stderr, "Error loading qrels: %s\n", err_buf);
        print_help(argv[0], stderr);
        goto ERROR;
    }

    qrels_set_reltype(qrels, opt.reltype, opt.reltype_arg);

    max_rel = qrels_get_max_rel(qrels);
    if (max_rel > 1.0) {
        warning("maximum effective relevance of %.2lf exceeds 1.0", max_rel);
    }

    res = evaluate_res(qrels, run, opt.ord, &opt.persist, &opt.depth);

    fmt_args.argc = argc;
    fmt_args.argv = argv;
    fmt_args.details = opt.details;
    fmt_args.opt = &opt;
    fmt_args.run = run;
    desc_fmt(res, &fmt_args, stdout);

    ret = 0;
    goto END;

ERROR:
    ret = -1;

END:
    if (qrels_fp)
        fclose(qrels_fp);
    if (run_fp)
        fclose(run_fp);
    if (run)
        run_delete(&run);
    if (qrels)
        qrels_delete(&qrels);
    if (res)
        res_delete(&res);

    return ret;
}
