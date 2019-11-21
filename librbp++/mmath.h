#ifndef RBPCC_MMATH_H
#define RBPCC_MMATH_H

namespace rbp {

/* Number of different ways of choosing $k$ elements from $n$.
 *
 * In other words, this calculates the binomial coefficient.
 */
unsigned long combinations(unsigned long n, unsigned long k); 

/* The number of different Kendall tau scores that can result
 * from a binary ordering of $k$ high values and $n-k$ low
 * values.
 *
 * Recall that Kendall's tau counts the number of swaps necessary
 * to order a list using bubblesort, normalised to the range 
 * [-1.0, 1.0].  Here, we simply calculate the maximum number
 * of swaps necessary to order a two-valued list, which occurs
 * when all the high values are at the start of the list.
 * If there are $k$ high values, then each value will $n - k$
 * swaps to get into the correct position, leading to a total
 * of $k(n-k)$ swaps
 */
inline unsigned long kendall_binary_selectivity(unsigned long n,
  unsigned long k) {
    return k * (n - k);
}

}; /* end namespace */

#endif /* RBPCC_MMATH_H */
