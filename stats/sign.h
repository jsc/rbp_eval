#ifndef SIGN_H
#define SIGN_H

/*
 *  Determine sign-test values and therefore sign-test significance.
 *
 *  The sign test on paired data is based on the proportion of paired
 *  items (x, y) from distributions X and Y in which x > y.  Were the
 *  two distributions "equal", then on average x will be greater than
 *  y half the time (ignoring ties).  The value calculated in the
 *  sign test is the proportion of times that x is greater than y.
 *  This is then compared with the results from a binomial distribution
 *  to determine significance.
 *
 *  Note that this is a one-tailed significance test.  That is, we
 *  are testing whether X is signficantly better than Y.  The p-value
 *  is returned.
 */
double paired_sign_test_p(double * dist_x, double * dist_y, unsigned dist_size,
  void * data);

#endif /* SIGN_H */
