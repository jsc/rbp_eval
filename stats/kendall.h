#ifndef KENDALL_H
#define KENDALL_H

/*
 *  Calculate Kendall's tau rank correlation between two lists.
 *
 *  Note that we do NOT adjust for ties (i.e. this is Kendall's
 *  tau-a, not tau-b or tau-c).
 */
double kendall_tau(double * dat1, double * dat2, unsigned dat_size);

#endif /* KENDALL_H */
