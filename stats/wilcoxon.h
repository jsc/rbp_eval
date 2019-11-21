#ifndef WILCOXON_H
#define WILCOXON_H

/*
 *  Determine significance by a one-tailed, paired Wilcoxon test.
 *
 *  We have two distributions X, Y, which are paired into (x, y)
 *  pairs, where each x, y is a measurement, and x - y is defined
 *  (e.g. x and y are scores, and x - y is the difference between
 *  scores).  Like the sign test, we examine in how many instances
 *  x > y, but here we take the magnitude of difference into account.
 *  We do not assume that X or Y are normally distributed.
 *
 *  The Wilcoxon test works as follows.  Derive Z from X, Y, where
 *  each z_i in Z is x_i - y_i in X, Y.  Rank the z values by absolute
 *  magnitude, averaging ranks amongst ties.  Restore signedness to
 *  ranks.  Sum up the positive signed ranks.  This gives us the
 *  Wilcoxon signed rank statistic $W^+$.
 */
double paired_wilcoxon_test_p(double * dist_x, double * dist_y,
  unsigned dist_size, void * data);

/*
 *  Generate a Wilcoxon distribution table for N values.
 *
 *  This is derived from:
 *
 *    http://comp9.psych.cornell.edu/Darlington/wilcoxon/wilcox4.htm
 *
 *  @param dist_size the size of the distribution
 *  @param outcomes the table to write the outcomes into.  This
 *    will give, for each W+ score, how many outcomes match that
 *    score.  For instance, index 0 says how many outcomes have
 *    a W+ of 0 (all elements fall below the median); index 1 says
 *    how many outcomes have a W+ of 1 (nearest element is above
 *    median, but all the rest are below); and so forth.  The
 *    number of possible positive outcomes 
 *    is (N(N+1)/2 + 1); it is up
 *    to the caller to make sure that @outcomes is at least this
 *    big.
 */
unsigned generate_wilcoxon_outcomes(unsigned dist_size,
  unsigned long long * outcomes_table);

#endif /* WILCOXON_H */
