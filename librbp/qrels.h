#ifndef QREL_H
#define QREL_H

#include <stdio.h>

#define REL_UNJUDGED    (-1.0)
#define REL_INVALID_QID (-2.0)

/* how to handle relevance scores in the qrels file. */
enum reltype_t {
    RELTYPE_BINARY,  /* collapse relevance to binary at a threshold */
    RELTYPE_FRACT,   /* allow fractional relevance scores, possibly
                        scaled. */
    RELTYPE_AUTO     /* figure out what to do from qrels file.  If only
                        integer relevances are found, then relevance will
                        be treated as binary at threshold 1.  If fractional
                        relevances are found, then relevance will be
                        treated as fractional. */
};

/**
 *  According to the ISO C standard, identifiers ending with _t are
 *  reserved for possible future additional type names.  However,
 *  we like living fast and loose.
 */
typedef double rel_t;

/**
 *  Object holding qrels for a query set.
 */
typedef struct qrels qrels_t;

/**
 *  Object holding qrels for an individual query.
 */
typedef struct qid_qrels qid_qrels_t;

/**
 *  Load qrels from an input stream.
 */
qrels_t * load_qrels(FILE * fp, char * err_buf, unsigned err_buf_len);

/**
 *  Get the number of qids in the qrels.
 */
unsigned qrels_get_num_qids(qrels_t * qrels);

/**
 *  Get a list of query ids.
 */
unsigned qrels_get_qids(qrels_t * qrels, const char ** qids_out,
  unsigned qids_out_size);

/**
 *  Set the relevance handling type for the qrels.
 */
void qrels_set_reltype(qrels_t * qrels, enum reltype_t reltype, 
  double reltype_arg);

/**
 *  Get the maximum (raw) relevance score.
 */
rel_t qrels_get_max_rel(qrels_t * qrels);

/**
 *  Get the relevance of a document to a query.
 *
 *  A graded relevance is returned, with 0.0 meaning "not at all
 *  relevant".  REL_UNJUDGED may also be returned.
 *
 *  @param qid the id of the topic (query).  Note that we represent
 *  this as a string, not an integer, as trec_eval does so.
 */
rel_t qrels_get_rel(qrels_t * qrels, const char * qid, const char * docid);

/**
 *  Get the number of documents relevant to a query.
 *
 *  If fractional relevances are supported, this is the sum of
 *  fractional relevances across all judgments.
 *
 *  < 0.0 indicates that there are no judgments for this query.
 */
double qrels_get_num_rel(qrels_t * qrels, const char * qid);

/**
 *  Get the qrels for a given qid.
 */
qid_qrels_t * qrels_get_qid_qrels(qrels_t * qrels, const char * qid);

/** 
 *  Delete a qrels structure.
 */
void qrels_delete(qrels_t ** qrels_p);

/**
 *  Get relevance of a given document for a qid.
 */
rel_t qid_qrels_get_rel(qid_qrels_t * qq, const char * docid);

/**
 *  Get the number of relevant documents for a qid.
 *
 *  If fractional relevances are allowed, this equates to the sum
 *  of fractional relevances across all judgments.
 */
double qid_qrels_get_num_rel(qid_qrels_t * qq);

/**
 *  Qrels iterator.
 */
typedef struct qrels_iterator qrels_iterator_t;

/**
 *  Get an iterator over judgments for this query.
 *
 *  Iteration order is arbitrary.
 */
qrels_iterator_t * qid_qrels_get_iterator(qid_qrels_t * qq);

/**
 *  Get the next relevance judgment in the iteration sequence.
 *
 *  The docid is returned, and its judged relevance is placed in rel.
 *  Note that rel is adjusted to the reltype for the qrels structure.
 */
const char * qrel_iter_next(qrels_iterator_t * iter, rel_t * rel);

void qrel_iter_delete(qrels_iterator_t ** iter_p);

#endif /* QREL_H */
