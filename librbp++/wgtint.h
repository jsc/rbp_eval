#ifndef RBPCC_WGTINT_H
#define RBPCC_WGTINT_H

#include <algorithm>

namespace rbp {

/**
 *  Calculate the intersection metric of Fagin et al. 7.2.
 *
 *  See Fagin et al., ``Comparing top $k$ lists'', SIAM (2003),
 *  section 7.2.  The intersection metric is a distance metric,
 *  with 1 meaning disjoint and 0 meaning identical.  The metric
 *  is calculated by averaging the proportional size of the
 *  set difference of the two lists at each successive depth:
 *
 *    (1/k) \sum_{r=1}^k |setdifference(prefix(S, r), prefix(K, r))|/2r
 *
 *  $depth$ specifies the maximum depth to go to; if left
 *  unspecified, then it is the shorter of the two lists.
 *  In any case, evaluation never goes deeper than the shorter
 *  list.
 */
template<class In> double wgtint(In start_a, In end_a, In start_b,
  In end_b, int depth=-1);

template<class In> double wgtint(In start_a, In end_a, In start_b,
  In end_b, int depth=-1) {
    int disjunction_size = 0;
    int rank;
    In a, b;
    double disj_sum = 0.0;

    for (a = start_a, b = start_b, rank = 0; 
      (depth < 0 || rank < depth) && a != end_a && b != end_b;
      a++, b++, rank++) {
        if (*a == *b) {
            /* no change */
        } else {
            if (std::find(start_b, b, *a) == b) {
                disjunction_size++; /* one more disjoint */
            } else {
                disjunction_size--; /* existing partner is matched */
            }
            if (std::find(start_a, a, *b) == a) {
                disjunction_size++; /* one more disjoint */
            } else {
                disjunction_size--; /* existing partner is matched */
            }
        }
        disj_sum += (double) disjunction_size / ((rank + 1) * 2);
    }
    return disj_sum / rank;
}

};

#endif /* RBPCC_WGTINT_H */
