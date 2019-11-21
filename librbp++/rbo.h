#ifndef RBPCC_RBO_H
#define RBPCC_RBO_H

#include <cmath>
#include <cassert>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <tr1/unordered_set>

namespace rbp {

/**
 *  Calculate the Rank-Biased Overlap of two rankings.
 *
 *  This in fact calculate the extrapolated RBO score.
 *  It does not handle ties (how do you specify ties in iterators?),
 *  but it does handle uneven lists.
 *
 *  We do not check for repeated items in either list.
 *
 *  @param depth the depth to evaluate RBO to; if non-positive,
 *      RBO is evaluated to the depth of the longer of the two
 *      lists.  If positive, it is evaluated to the lesser of
 *      the longer of the two lists and the specified depth.
 */
template<class In, class Item> double rbo(double p, In start_a, In end_a, 
  In start_b, In end_b, int depth=-1) {
    int overlap = 0, short_overlap = -1;
    int rank, short_len;
    In a, b;
    double rbo_sum = 0.0, rbo_score;
    std::tr1::unordered_set<Item> intersect;

    for (a = start_a, b = start_b, rank = 0, short_len = 0;
      (a != end_a || b != end_b) && (depth <= 0 || rank < depth); rank++) {
        if (a != end_a && b != end_b) {
            short_len++;
        } else if (short_overlap == -1) {
            short_overlap = overlap;
        }
        if (a != end_a && b != end_b && *a == *b) {
            overlap++;
        } else {
            if (a != end_a) {
                if (intersect.find(*a) != intersect.end()) {
                    overlap++;
                } else {
                    intersect.insert(*a);
                }
            } 
            if (b != end_b) {
                if (intersect.find(*b) != intersect.end()) {
                    overlap++;
                } else {
                    intersect.insert(*b);
                }
            }
        }
        if (a != end_a) {
            a++;
        } 
        if (b != end_b) {
            b++;
        }

        assert(overlap <= (rank + 1));
        double rbo_contr = (double) overlap / (rank + 1);
        if (short_len < (rank + 1)) {
            rbo_contr += (double) short_overlap * (rank + 1 - short_len) 
                 / (short_len * (rank + 1));
        }
        rbo_sum += rbo_contr * pow(p, rank);
    }
    rbo_score = rbo_sum * (1 - p);
    /* rbo_score above gives the rbo@k score.  Now we extrapolate the 
       current level of agreement out to infinity. */
    if (short_len < rank) {
        double tail = pow(p, rank) * (((double) short_overlap / short_len) +
          ((double) overlap - short_overlap) / rank);
        rbo_score += tail;
    } else {
        rbo_score += pow(p, rank) * overlap / rank;
    }
    assert((rbo_score - 1.0) < 0.001);
    assert(rbo_score >= 0.0);
    return rbo_score;
}

};



#endif /* RBPCC_RBO_H */
