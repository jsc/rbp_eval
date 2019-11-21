#ifndef RBPCC_KENDPNLT_H
#define RBPCC_KENDPNLT_H

#include <algorithm>
#include <vector>
#include <cassert>
extern "C" {
#include <limits.h>
}

namespace rbp {

/**
 *  Calculate Kendall's distance with penalty parameter.
 *
 *  See Fagin et al., ``Comparing top $k$ lists'', SIAM (2003).  We'll
 *  refer to the metric as K^(p) or KP.  KP is formulated as a distance
 *  metric (0 is identical, higher means further away), and as 
 *  described in the paper is non-normalised.  KP is based on 
 *  Kendall's tau.  For completely non-disjoint lists it _IS_
 *  Kendall's tau unnormalised, that is, the straight count of
 *  discordant pairs.  Partially disjoint lists are handled
 *  as described below.
 *
 *  Call our lists S and T, and our elements a and b.
 *  s_a, t_a are the positions of a in S, T, and similarly
 *  for b.  Below, we only consider one of each symmetrical
 *  case.
 *
 *  1. If a in S, T, and b in S, T, handle as for regular Kendall's
 *  tau.
 *
 *  2. If a in S, T; b in S but not in T: concordant if s_a > s_b,
 *  discordant otherwise.
 *
 *  3. If a in S but not in T, b in T but not in S: discordant.
 *
 *  4. If a, b in S; a, b not in T: apply penalty $p$; this is the
 *  only case where the cordance of S, T is ambiguous.
 *
 *  The score is then normalised to the range [-1, 1], with
 *  -1 meaning completely disjoint and 1 meaning completely
 *  identical.  Note that conjoint but reversed is not -1,
 *  nor (necessarily) 0, but somewhere between [0, 1],
 *  depending on the penalty.  With lists both of length $k$,
 *  the distance for conjoint but reversed is $d(k) = ((k)(k - 1))/2$,
 *  as for regular Kendall's tau.   The distance for full
 *  disjointness is $(k ^ 2) + (2 * d(k) * penalty)$.  Thus, after
 *  normalisation, the greater the penalty, the closer conjoint
 *  but reversed is to identical.  For this reason, we set the
 *  default penalty to $0.0$, as suggested by Fagin et al.
 *
 *  Where the lists are of different length, $k1$ and $k2$,
 *  the distance for full disjointness is 
 *  $(k1 ^ k2) + (d(k1) + d(k2)) * penalty$.
 *
 *  $penalty$ should be a value from 0.0 (minimise) to 1.0
 *  (maximise), although in practice the highest value would
 *  be 0.5 (neutral).
 *
 *  $depth$ is the maximum depth to calculate the metric to.  It defaults
 *  to the longer of the two lists.
 */
template<class In> double kendpenalty(In start_a, In end_a,
  In start_b, In end_b, double penalty=0.0, int depth=-1);

template<class In> double kendpenalty(In start_a, In end_a,
  In start_b, In end_b, double penalty=0.0, int depth=-1) {
    /* The implementation is to treat $a$ as the gold standard,
     * then convert $b$ into a list of integers giving the
     * elements their respective ranks in $a$.  Elements in
     * $b$ that are not in $a$ are marked -1.  Elements that
     * are in $a$ but not in $b$ are appended and marked -2.
     */
    const int not_in_a = 0;
    std::vector<int> b_ord;
    std::vector<int>::iterator b1, b2;
    In b, a;
    double discordance = 0.0;
    int len_a, len_b;
    int pos, d, wk_depth;

    if (depth <= 0)
        wk_depth = INT_MAX;
    else
        wk_depth = depth;
    for (d = 0, b = start_b; b != end_b && d < wk_depth; b++, d++) {
        /* We do the forward search ourselves because we want to support
         * simple Input iterators (well, not really, but it doesn't hurt)
         * and the std::find algorithm doesn't give us the index. 
         * We count positions from 1 because 0 will mean "not in a" and
         * -ve will mean "in a, but not in b". */
        for (a = start_a, pos = 1; a != end_a && *a != *b 
          && (pos - 1) < wk_depth; a++, pos++)
            ;
        if (a == end_a || (pos - 1) == wk_depth) {
            b_ord.push_back(not_in_a);
        } else {
            b_ord.push_back(pos);
        }
    }
    assert(b == end_b || d == wk_depth);
    len_b = b_ord.size();
    for (a = start_a, pos = 1; a < end_a && (pos - 1) < wk_depth; a++, pos++) {
        if (std::find(start_b, b, *a) == b) {
            b_ord.push_back(-pos);
        }
    }
    len_a = pos - 1;

    /* Hokey, now we just have to calculate discordance over the
     * pairs in b_ord. */
    for (b1 = b_ord.begin(); b1 != b_ord.end(); b1++) {
        for (b2 = b1 + 1; b2 != b_ord.end(); b2++) {
            if (*b1 == not_in_a) {
                assert(b1 - b_ord.begin() < len_b);
                if (*b2 == not_in_a) {
                    /* pair in B, not in A */
                    /* A: ? ?; B: 1 2 */
                    assert(b2 - b_ord.begin() < len_b);
                    discordance += penalty;
                } else if (*b2 < 0) {
                    /* 1 in B, not A; 2 in A, not B */
                    /* A: 2 ?; B: 1 ? */
                    assert(b2 - b_ord.begin() >= len_b);
                    discordance += 1.0;
                } else {
                    /* 1 in B, not in A; 2 in both */
                    /* A: 2 ?; B: 1 2 */
                    assert(b2 - b_ord.begin() < len_b);
                    discordance += 1.0;
                }
            } else if (*b1 < 0) {
                assert(b1 - b_ord.begin() >= len_b);
                assert(*b2 < 0);
                /* pair in A, not in B */
                /* A: 1 2; B: ? ? */
                discordance += penalty;
            } else {
                assert(b1 - b_ord.begin() < len_b);
                if (*b2 == not_in_a) {
                    /* 1 in A and B, 2 in B. */
                    /* A: 1 ?; B: 1 2 */
                    assert(b2 - b_ord.begin() < len_b);
                    //discordance += 1.0;
                } else if (*b2 < 0) {
                    /* 1 in A and B, 2 in A. */
                    /* Check order of 1, 2 in A. */
                    assert(b2 - b_ord.begin() >= len_b);
                    if (*b1 > -(*b2))
                        discordance += 1.0;
                } else {
                    /* 1 in A and B, 2 in A and B; compare order. */
                    assert(b2 - b_ord.begin() < len_b);
                    if (*b1 > *b2)
                        discordance += 1.0;
                }
            }
        }
    }
    /* The maximum distance is for disjoint sets; see the formula
     * in the header comment. */
    double max_dist = len_a * len_b + 
        (len_a * (len_a - 1) + len_b * (len_b - 1)) * penalty / 2;

    /* Normalise so that 0 -> 1, max_dist -> -1 */
    return 1 - (2 * discordance / max_dist);
}

};

#endif /* RBPCC_KENDPNLT_H */
