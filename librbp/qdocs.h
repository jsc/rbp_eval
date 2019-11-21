#ifndef QDOCS_H
#define QDOCS_H

typedef struct qdocs qdocs_t;

typedef struct doc_score {
    char * docid;
    unsigned occur;
    unsigned rank;
    double score;
    unsigned flags;
} doc_score_t;

enum qdocs_ord_t {
    QDOCS_ORD_OCCUR,
    QDOCS_ORD_RANK,
    QDOCS_ORD_SCORE
};

/* order by similarity score, as with trec_eval.
 * ties are nominally ordered by docid, but in fact rbp_eval handles
 * ties explicitly. */
#define QDOCS_DEFAULT_ORDERING QDOCS_ORD_SCORE

qdocs_t * new_qdocs(char * qid);

void qdocs_delete(qdocs_t ** qd_p);

void qdocs_add_doc_score(qdocs_t * qd, char * docid, unsigned rank,
  double score);

unsigned qdocs_num_scores(qdocs_t * qd);

char * qdocs_qid(qdocs_t * qd);

doc_score_t * qdocs_get_scores(qdocs_t * qd, enum qdocs_ord_t ord);

#endif /* QDOCS_H */
