#include "trec_fmt.h"
#include "error.h"

#define NA_STR "-"

#define TR_LABEL_COL_WIDTH   12
#define TR_QID_COL_WIDTH      6
#define TR_VAL_COL_WIDTH     12
#define TR_DEPTH_COL_WIDTH    6
#define TR_PERSIST_COL_WIDTH  6

#define LABEL_FMT "%-*s"
#define QID_FMT "%*s"
#define PERSIST_FMT "%*.2lf"
#define DEPTH_FMT "%*u"
#define RBP_FMT "%*.4lf"
#define RBPERR_FMT "%*.4lf"

#define NA   -1
#define OMIT -2

#define D_NA   -1.0
#define D_OMIT -2.0


static void trec_eval_rel_qid(qid_res_t * qres, depth_t * depth,
  persist_t * persist, FILE * fp); 

void trec_fmt(res_t * res, fmt_args_t * args, FILE * fp) {
    unsigned q;
    enum details_t details = args->details;

    for (q = 0; q < res->num_qid; q++) {
        qid_res_t * qres;

        qres = &res->qid_res[q];
        if (qres->num_rel < 0) {
            warning("there are no judgments for query id %s", qres->qid);
            continue;
        }
        if (details & DETAILS_PER_QUERY) {
            trec_eval_rel_qid(qres, res->depth, res->persist, fp);
        }
    }
    if (details & DETAILS_AVERAGES)
        trec_eval_rel_qid(&res->ave_res, res->depth, res->persist, fp);
}

#define PRINT_TR_ROW(fp, label, qid, val_fmt, val, depth, persist) {  \
    fprintf(fp, LABEL_FMT, TR_LABEL_COL_WIDTH, label);                \
    fprintf(fp, " " QID_FMT, TR_QID_COL_WIDTH, qid);                  \
    if ((depth) == NA)                                                \
        fprintf(fp, " %*s", TR_DEPTH_COL_WIDTH, NA_STR);              \
    else if ((depth) != OMIT)                                         \
        fprintf(fp, " " DEPTH_FMT, TR_DEPTH_COL_WIDTH, depth);        \
    if ((persist) == D_NA)                                            \
        fprintf(fp, " %*s", TR_PERSIST_COL_WIDTH, NA_STR);            \
    else if ((persist) != D_OMIT)                                     \
        fprintf(fp, " " PERSIST_FMT, TR_PERSIST_COL_WIDTH, persist);  \
    fprintf(fp, " " val_fmt, TR_VAL_COL_WIDTH, val);                  \
    fprintf(fp, "\n");                                                \
}

static void trec_eval_rel_qid(qid_res_t * qres, depth_t * depth,
  persist_t * persist, FILE * fp) {
    unsigned d, p;
    PRINT_TR_ROW(fp, "num_ret", qres->qid, "%*u", qres->num_ret,
      (depth->d_num > 1) ? NA : OMIT, (persist->p_num > 1) ? D_NA : D_OMIT); 
    PRINT_TR_ROW(fp, "num_rel", qres->qid, "%*.1lf", qres->num_rel,
      (depth->d_num > 1) ? NA : OMIT, (persist->p_num > 1) ? D_NA : D_OMIT); 
    for (d = 0; d < depth->d_num; d++) {
        depth_res_t * dres;

        dres = &qres->depth_res[d];
        PRINT_TR_ROW(fp, "num_rel_ret", qres->qid, "%*.1lf", dres->num_rel_ret,
          (depth->d_num > 1) ? depth->d[d] : OMIT, 
          (persist->p_num > 1) ? D_NA : D_OMIT); 
        for (p = 0; p < persist->p_num; p++) {
            PRINT_TR_ROW(fp, "rbp", qres->qid, RBP_FMT, 
              dres->persist_res[p].sum,
              (depth->d_num > 1) ? depth->d[d] : OMIT, 
              (persist->p_num > 1) ? persist->p[p] : D_OMIT); 
            PRINT_TR_ROW(fp, "rbperr", qres->qid, RBPERR_FMT, 
              dres->persist_res[p].err,
              (depth->d_num > 1) ? depth->d[d] : OMIT, 
              (persist->p_num > 1) ? persist->p[p] : D_OMIT); 
        }
    }
}
