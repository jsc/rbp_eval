#ifndef BOOTSTRAP_H
#define BOOTSTRAP_H

/*
 *  Calculate p-value for a paired, one-tailed bootstrap test.
 *
 *  The test statistic is mean.  (I know Sakai proposes a
 *  "studentized" test statistic as giving greater accuracy, but
 *  I don't understand _why_ it gives greater accuracy...)
 */
double paired_bootstrap_test_p(double * dist_x, double * dist_y,
  unsigned dist_size, void * data);

#endif /* BOOTSTRAP_H */
