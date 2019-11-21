#ifndef RBP_H
#define RBP_H

#include "qrels.h"
#include "qdocs.h"
#include "depth.h"
#include "persist.h"

typedef struct rbp rbp_t;

typedef struct {
    double sum;
    double cumerr;
    double wgt;
    double tie_wgt;
    double err;
} rbp_val_t;

rbp_t * new_rbp(qrels_t * qrels, enum qdocs_ord_t ord,
  persist_t * persist);

/*
 *  (Re-)Initialise for processing of a ranking for a particular query.
 */
int rbp_init(rbp_t * rbp, qdocs_t * qdocs);

/*
 *  Perform rbp calculation.
 */
rbp_val_t * rbp_calc_to_depth(rbp_t * rbp, unsigned depth, 
  double * num_rel_ret);

/*
 *  Calculate rbp weights at each depth for a given persistence.
 *
 *  PERSIST is the persistence ('p') value to use in calculating
 *  the rbp weights.  WGTS is an array into which the weights will 
 *  be written.  DEPTH is the depth to calculate weights to.
 *  Obviously, len(WGTS) should be >= DEPTH.
 */
void rbp_weights(double * wgts, double persist, unsigned depth);

/*
 *  Calculate the rbp value at a particular point.
 */
double rbp(double persist, unsigned rank);

void rbp_delete(rbp_t ** rbp_p);


#endif /* RBP_H */
