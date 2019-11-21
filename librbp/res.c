#include "res.h"
#include "rbp.h"
#include "util.h"
#include <assert.h>

static void _init_qid_res(qid_res_t * qres, unsigned d_num, unsigned p_num);
static void _cleanup_qid_res(qid_res_t * qres, unsigned d_num);
static res_t * _new_res(unsigned num_qid, depth_t * depth, persist_t * persist);

res_t * evaluate_res(qrels_t * qrels, run_t * run, enum qdocs_ord_t ord,
  persist_t * persist, depth_t * depth) {
    res_t * res;
    rbp_t * rbp;
    unsigned q, d, p;
    unsigned num_judged_queries;
    qid_res_t * ave;

    res = _new_res(run_num_qdocs(run), depth, persist);
    ave = &res->ave_res;
    rbp = new_rbp(qrels, ord, persist);
    num_judged_queries = 0;
    for (q = 0; q < res->num_qid; q++) {
        qdocs_t * qd;
        char * qid;
        qid_res_t * qres;
        int ret;
        
        qd = run_get_qdocs_by_index(run, q);
        qid = qdocs_qid(qd);
        qres = &res->qid_res[q];
        qres->qid = qid;
        qres->num_ret = qdocs_num_scores(qd);
        qres->num_rel = qrels_get_num_rel(qrels, qid);
        if (qres->num_rel == -1) {
            continue;
        }
        num_judged_queries++;
        ave->num_rel += qres->num_rel;
        ave->num_ret += qres->num_ret;
        ret = rbp_init(rbp, qd);
        assert(ret == 0);
        for (d = 0; d < depth->d_num; d++) {
            rbp_val_t * rbpvals;
            depth_res_t * dres;

            dres = &qres->depth_res[d];
            rbpvals = rbp_calc_to_depth(rbp, depth->d[d], &dres->num_rel_ret);
            ave->depth_res[d].num_rel_ret += dres->num_rel_ret;
            for (p = 0; p < persist->p_num; p++) {
                dres->persist_res[p].sum = rbpvals[p].sum;
                dres->persist_res[p].err = rbpvals[p].err;
                ave->depth_res[d].persist_res[p].sum += rbpvals[p].sum;
                ave->depth_res[d].persist_res[p].err += rbpvals[p].err;
            }
        }
    }
    /* calculate averages */
    for (d = 0; d < depth->d_num; d++) {
        for (p = 0; p < persist->p_num; p++) {
            ave->depth_res[d].persist_res[p].sum /= num_judged_queries;
            ave->depth_res[d].persist_res[p].err /= num_judged_queries;
        }
    }
    rbp_delete(&rbp);
    return res;
}

void res_delete(res_t ** res_p) {
    res_t * res = *res_p;
    unsigned q;

    _cleanup_qid_res(&res->ave_res, res->depth->d_num);
    for (q = 0; q < res->num_qid; q++) {
        _cleanup_qid_res(&res->qid_res[q], res->depth->d_num);
    }
    free(res->qid_res);
    free(res);
    *res_p = NULL;
}

static res_t * _new_res(unsigned num_qid, depth_t * depth, 
  persist_t * persist) {
    res_t * res;
    unsigned q;

    res = util_malloc_or_die(sizeof(*res));
    _init_qid_res(&res->ave_res, depth->d_num, persist->p_num);
    res->ave_res.qid = "all";
    res->qid_res = util_malloc_or_die(sizeof(*res->qid_res) * num_qid);
    for (q = 0; q < num_qid; q++) {
        _init_qid_res(&res->qid_res[q], depth->d_num, persist->p_num);
    }
    res->num_qid = num_qid;
    res->depth = depth;
    res->persist = persist;
    return res;
}

static void _init_qid_res(qid_res_t * qres, unsigned d_num, unsigned p_num) {
    unsigned d, p;

    qres->qid = NULL;
    qres->num_ret = qres->num_rel = 0;
    qres->depth_res = util_malloc_or_die(sizeof(*qres->depth_res) * d_num);
    for (d = 0; d < d_num; d++) {
        depth_res_t * dres = &qres->depth_res[d];
        dres->num_rel_ret = 0;
        dres->persist_res = util_malloc_or_die(sizeof(*dres->persist_res)
          * p_num);
        for (p = 0; p < p_num; p++) {
            dres->persist_res[p].sum = 0.0;
            dres->persist_res[p].err = 0.0;
        }
    }
}

static void _cleanup_qid_res(qid_res_t * qres, unsigned d_num) {
    unsigned d;

    for (d = 0; d < d_num; d++) {
        free(qres->depth_res[d].persist_res);
    }
    free(qres->depth_res);
}
