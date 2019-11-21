#ifndef STATS_H
#define STATS_H

/*
 *  Common stats interfaces and utilities.
 */

/*
 *  Interface for one-tailed, paired significance test.
 *
 *  Returns the likelihood that difference could have happened by chance, that is,
 *  the likelihood that the mean of the first population is not higher than that
 *  of the second.
 */
typedef double (*paired_test_p_fn_t)(double * dist_x, double * dist_y, unsigned dist_size,
  void * data);


#endif /* STATS_H */
