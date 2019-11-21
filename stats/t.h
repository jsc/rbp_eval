#ifndef T_H
#define T_H

/*
 *  Determine t value of one-tailed, paired T-test.
 *
 *  The function returns the "t" value.
 */
double paired_t_test(double * dist_x, double * dist_y, unsigned dist_size);

/*
 *  Determine significane of one-tailed, paired T-test.
 *
 *  The function does not return the exact p value, but the value
 *  from the lookup-table that the calculate t value exceeds.
 */
double paired_t_test_p(double * dist_x, double * dist_y, unsigned dist_size,
  void * data);

#endif /* T_H */
