#include <errno.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "util.h"
#include "strhash.h"
#include "qrels.h"
#include "error.h"

#define LINE_BUF_SZ 1024
#define QID_BUF_SZ 256

#define QREL_NUM_COLS 4
#define QREL_QID_COL 0
#define QREL_ITER_COL 1
#define QREL_DOCID_COL 2
#define QREL_REL_COL 3

struct qrel {
    const char * docid;
    double rel;
};

struct qrels {
    strhash_t * tbl;
    int all_rels_are_integral;
    enum reltype_t reltype;
    double reltype_arg;
    rel_t max_rel;
};

struct qid_qrels {
    double raw_num_rel;
    strhash_t * tbl;
    struct qrels * qrels;
};

struct qrels_iterator {
    strhash_iter_t * sh_iter;
    struct qid_qrels * qq;
};

struct qrel_line_pos {
    char * qid, * iter, * docid, * rel;
};

struct count_rel_data {
    qrels_t * qr;
    double count;
};

static qrels_t * _new_qrels();
static int _parse_qrel_line(char * line, struct qrel_line_pos * pos);
static struct qid_qrels * _new_qid_qrels(struct qrels * qrels);
static struct qid_qrels * _get_qid_qrels(qrels_t * qrels, char * qid);
static void _delete_qid_qrels(void * dat);
static void _count_rel(const char * docid, strhash_data_t rel, void * userdata);
static rel_t _adj_rel(qrels_t * qr, rel_t raw_rel);

qrels_t * load_qrels(FILE * fp, char * err_buf, unsigned err_buf_len) {
    char line_buf[LINE_BUF_SZ];
    char qid_buf[QID_BUF_SZ];
    struct qid_qrels * qq = NULL;
    unsigned line_num = 0;
    qrels_t * qr;

    qid_buf[0] = '\0';
    qr = _new_qrels();
    while (fgets(line_buf, LINE_BUF_SZ, fp) != NULL) {
        int ret;
        struct qrel_line_pos pos;
        rel_t rel;
        char * relend;
        strhash_data_t * reldatp;
        int found;

        line_num++;
        ret = _parse_qrel_line(line_buf, &pos);
        if (ret < 0) {
            if (err_buf)
                snprintf(err_buf, err_buf_len, 
                  "wrong number of fields on line %d of qrels file", line_num);
            qrels_delete(&qr);
            return NULL;
        }
        rel = strtod(pos.rel, &relend);
        if (rel < 0.0 || *relend != '\0') {
            if (err_buf)
                snprintf(err_buf, err_buf_len,
                  "rel not a non-negative float on line %d of qrels file",
                  line_num);
            qrels_delete(&qr);
            return NULL;
        }
        if (rel > qr->max_rel)
            qr->max_rel = rel;
        if (qr->all_rels_are_integral && strchr(pos.rel, '.') != NULL) {
            qr->all_rels_are_integral = 0;
        }
        util_downcase_str(pos.qid);
        util_downcase_str(pos.docid);
        if (strcmp(pos.qid, qid_buf) != 0) {
            strncpy(qid_buf, pos.qid, QID_BUF_SZ);
            qid_buf[QID_BUF_SZ - 1] = '\0';
            qq = _get_qid_qrels(qr, pos.qid);
        }
        reldatp = strhash_update(qq->tbl, pos.docid, &found);
        if (found) {
            if (err_buf)
                snprintf(err_buf, err_buf_len,
                  "duplicate relevance for qid '%s' and docid '%s' on line "
                  "'%d'", pos.qid, pos.docid, line_num);
            qrels_delete(&qr);
            return NULL;
        }
        if (rel > 0.0) {
            qq->raw_num_rel += rel;
        }
        reldatp->lf = rel;
    }
    return qr;
}

unsigned qrels_get_num_qids(qrels_t * qrels) {
    return strhash_num_entries(qrels->tbl);
}

static int qid_cmp(const void * a, const void * b) {
    char * qa = *(char **) a;
    char * qb = *(char **) b;
    int ia, ib;
    char * endptr;
    ia = strtoul(qa, &endptr, 10);
    if (*endptr == '\0') {
        ib = strtoul(qb, &endptr, 10);
        if (*endptr == '\0') {
            return ia - ib;
        }
    }
    return strcmp(qa, qb);
}

unsigned qrels_get_qids(qrels_t * qrels, const char ** qids_out,
  unsigned qids_out_size) {
    strhash_iter_t * iter = strhash_get_iter(qrels->tbl);
    unsigned q;

    for (q = 0; q < qids_out_size && 
      (qids_out[q] = strhash_iter_next(iter, NULL)) != NULL; q++)
        ;
    strhash_iter_delete(&iter);
    qsort(qids_out, q, sizeof(*qids_out), qid_cmp);
    return q;
}

void qrels_set_reltype(qrels_t * qrels, enum reltype_t reltype,
  double reltype_arg) {
    qrels->reltype = reltype;
    qrels->reltype_arg = reltype_arg;
}

void qrels_delete(qrels_t ** qrels_p) {
    qrels_t * qrels = *qrels_p;
    strhash_delete(&qrels->tbl, _delete_qid_qrels);
    free(qrels);
    *qrels_p = NULL;
}

rel_t qrels_get_max_rel(qrels_t * qrels) {
    return _adj_rel(qrels, qrels->max_rel);
}

rel_t qrels_get_rel(qrels_t * qrels, const char * qid, const char * docid) {
    qid_qrels_t * qq;

    qq = qrels_get_qid_qrels(qrels, qid);
    if (qq == NULL)
        return REL_INVALID_QID;
    else
        return qid_qrels_get_rel(qq, docid);
}

double qrels_get_num_rel(qrels_t * qrels, const char * qid) {
    qid_qrels_t * qq;
    qq = qrels_get_qid_qrels(qrels, qid);
    if (qq == NULL)
        return -1.0;
    return qid_qrels_get_num_rel(qq);
}

qid_qrels_t * qrels_get_qid_qrels(qrels_t * qrels, const char * qid) {
    return (qid_qrels_t *) strhash_get(qrels->tbl, qid, NULL).v;
}

rel_t qid_qrels_get_rel(qid_qrels_t * qq, const char * docid) {
    strhash_data_t rel;
    int found;

    rel =  strhash_get(qq->tbl, docid, &found);
    if (!found) {
        return REL_UNJUDGED;
    } else {
        return _adj_rel(qq->qrels, rel.lf); 
    }
}

double qid_qrels_get_num_rel(qid_qrels_t * qq) {
    struct count_rel_data dat;
    dat.qr = qq->qrels;
    dat.count = 0.0;
    strhash_foreach(qq->tbl, _count_rel, &dat);
    return dat.count;
}

static qrels_t * _new_qrels() {
    qrels_t * qr;
    qr = util_malloc_or_die(sizeof(*qr));
    qr->tbl = new_strhash();
    qr->all_rels_are_integral = 1;
    qr->reltype = RELTYPE_AUTO;
    qr->reltype_arg = 1.0;
    qr->max_rel = 0.0;
    return qr;
}

static int _parse_qrel_line(char * line, struct qrel_line_pos * pos) {
    char * cols[QREL_NUM_COLS];
    int ret;

    ret = util_parse_cols(line, cols, QREL_NUM_COLS);
    if (ret < 0)
        return ret;

    pos->qid = cols[QREL_QID_COL];
    pos->iter = cols[QREL_ITER_COL];
    pos->docid = cols[QREL_DOCID_COL];
    pos->rel = cols[QREL_REL_COL];
    return 0;
}

static struct qid_qrels * _new_qid_qrels(struct qrels * qrels) {
    struct qid_qrels * qq;
    qq = util_malloc_or_die(sizeof(*qq));
    qq->tbl = new_strhash();
    qq->raw_num_rel = 0;
    qq->qrels = qrels;
    return qq;
}

static struct qid_qrels * _get_qid_qrels(qrels_t * qrels, char * qid) {
    struct qid_qrels ** qqp;
    qqp = (struct qid_qrels **) strhash_update(qrels->tbl, qid, NULL);
    if (*qqp == NULL) {
        *qqp = _new_qid_qrels(qrels);
    }
    return *qqp;
}

static void _delete_qid_qrels(void * dat) {
    struct qid_qrels * qq = dat;
    strhash_delete(&qq->tbl, NULL);
    free(qq);
}

static void _count_rel(const char * docid, strhash_data_t rel,
  void * userdata) {
    struct count_rel_data * rel_data = userdata;
    rel_data->count += _adj_rel(rel_data->qr, rel.lf);
}

static rel_t _adj_rel(qrels_t * qr, rel_t raw_rel) {
    switch (qr->reltype) {
    case RELTYPE_AUTO:
        if (qr->all_rels_are_integral) {
            goto RELTYPE_BINARY;
        } else {
            goto RELTYPE_FRACT;
        }
        break;
RELTYPE_BINARY:
    case RELTYPE_BINARY:
        if (raw_rel >= qr->reltype_arg)
            return 1.0;
        else
            return 0.0;
        break;
RELTYPE_FRACT:
    case RELTYPE_FRACT:
        return raw_rel * qr->reltype_arg;
    } 
    assert(0);
    return 0.0;
}

qrels_iterator_t * qid_qrels_get_iterator(qid_qrels_t * qq) {
    qrels_iterator_t * qit = util_malloc_or_die(sizeof(*qit));
    qit->sh_iter = strhash_get_iter(qq->tbl);
    qit->qq = qq;
    return qit;
}

const char * qrel_iter_next(qrels_iterator_t * iter, rel_t * rel) {
    strhash_data_t dat;
    const char * docid = NULL;

    docid =  strhash_iter_next(iter->sh_iter, &dat);
    if (docid == NULL)
        return NULL;
    *rel = _adj_rel(iter->qq->qrels, dat.lf);
    return docid;
}

void qrel_iter_delete(qrels_iterator_t ** iter_p) {
    strhash_iter_delete(&(*iter_p)->sh_iter);
    free(*iter_p);
    *iter_p = NULL;
}

#ifdef QRELS_MAIN

int main(int argc, char ** argv) {
    char * fname;
    FILE * fp;
    qrels_t * qrels;
    struct qrel_line_pos pos;
    char line_buf[LINE_BUF_SZ];

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <qrels-file>\n", argv[0]);
        return -1;
    }
    fname = argv[1];
    fp = fopen(fname, "r");
    if (fp == NULL) {
        fprintf(stderr, "Unable to open file %s for reading\n", fname);
        return -1;
    }
    qrels = load_qrels(fp, NULL, 0);
    if (qrels == NULL) {
        fprintf(stderr, "Error loading qrels file\n");
        fclose(fp);
        return -1;
    }
    rewind(fp);
    while (fgets(line_buf, LINE_BUF_SZ, fp) != NULL) {
        qid_qrels_t * qq;
        qrels_iterator_t * qit;
        rel_t rel;

        _parse_qrel_line(line_buf, &pos);
        rel = qrels_get_rel(qrels, pos.qid, pos.docid);
        assert(rel == strtod(pos.rel, NULL));
        assert(qrels_get_rel(qrels, pos.qid, "no-docid-like-this-i-hope")
          == REL_UNJUDGED);
        assert(qrels_get_rel(qrels, "no-qid-like-this-i-hope", pos.docid)
          == REL_INVALID_QID);
        qq = qrels_get_qid_qrels(qrels, pos.qid);
        qit = qid_qrels_get_iterator(qq);
        while ( (qrel_iter_next(qit, &rel)) != NULL) {
        }
        qrel_iter_delete(&qit);
    }
    qrels_delete(&qrels);
    fclose(fp);
    return 0;
}

#endif /* QRELS_MAIN */
