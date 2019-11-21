#include <math.h>
#include "t.h"
#include "util.h"

#include <stdio.h>

/*
    Table of significance values for t.

    First column is degrees of freedom.
    Next columns give one-tailed t thresholds for p=:

             0.1   0.05   0.025 0.005 0.0025 0.0005  0.00025 0.00005

    For two-tailed, double the p values.  (These column p-values
    are given is

    The actual formula for the CDF of the Student's-t distribution
    is very complicated, so we don't calculate it here.  (If we
    wanted to, we could include the nmath library from R, which
    does calculate the p value...)
 */
static double t_significance_table[][9] = {
      { 2,  1.89,  2.92,  4.30,  9.92, 14.09, 31.60, 44.70, 100.14 },
      { 3, 1.64, 2.35, 3.18, 5.84, 7.45, 12.92, 16.33, 28.01 },
      { 4, 1.53, 2.13, 2.78, 4.60, 5.60, 8.61, 10.31, 15.53 },
      { 5, 1.48, 2.02, 2.57, 4.03, 4.77, 6.87, 7.98, 11.18 },
      { 6, 1.44, 1.94, 2.45, 3.71, 4.32, 5.96, 6.79, 9.08 },
      { 7, 1.41, 1.89, 2.36, 3.50, 4.03, 5.41, 6.08, 7.89 },
      { 8, 1.40, 1.86, 2.31, 3.36, 3.83, 5.04, 5.62, 7.12 },
      { 9, 1.38, 1.83, 2.26, 3.25, 3.69, 4.78, 5.29, 6.59 },
      { 10, 1.37, 1.81, 2.23, 3.17, 3.58, 4.59, 5.05, 6.21 },
      { 11, 1.36, 1.80, 2.20, 3.11, 3.50, 4.44, 4.86, 5.92 },
      { 12, 1.36, 1.78, 2.18, 3.05, 3.43, 4.32, 4.72, 5.70 },
      { 13, 1.35, 1.77, 2.16, 3.01, 3.37, 4.22, 4.60, 5.51 },
      { 14, 1.35, 1.76, 2.14, 2.98, 3.33, 4.14, 4.50, 5.36 },
      { 15, 1.34, 1.75, 2.13, 2.95, 3.29, 4.07, 4.42, 5.24 },
      { 16, 1.34, 1.75, 2.12, 2.92, 3.25, 4.01, 4.35, 5.13 },
      { 17, 1.33, 1.74, 2.11, 2.90, 3.22, 3.97, 4.29, 5.04 },
      { 18, 1.33, 1.73, 2.10, 2.88, 3.20, 3.92, 4.23, 4.97 },
      { 19, 1.33, 1.73, 2.09, 2.86, 3.17, 3.88, 4.19, 4.90 },
      { 20, 1.33, 1.72, 2.09, 2.85, 3.15, 3.85, 4.15, 4.84 },
      { 21, 1.32, 1.72, 2.08, 2.83, 3.14, 3.82, 4.11, 4.78 },
      { 22, 1.32, 1.72, 2.07, 2.82, 3.12, 3.79, 4.08, 4.74 },
      { 23, 1.32, 1.71, 2.07, 2.81, 3.10, 3.77, 4.05, 4.69 },
      { 24, 1.32, 1.71, 2.06, 2.80, 3.09, 3.75, 4.02, 4.65 },
      { 25, 1.32, 1.71, 2.06, 2.79, 3.08, 3.73, 4.00, 4.62 },
      { 26, 1.31, 1.71, 2.06, 2.78, 3.07, 3.71, 3.97, 4.59 },
      { 27, 1.31, 1.70, 2.05, 2.77, 3.06, 3.69, 3.95, 4.56 },
      { 28, 1.31, 1.70, 2.05, 2.76, 3.05, 3.67, 3.93, 4.53 },
      { 29, 1.31, 1.70, 2.05, 2.76, 3.04, 3.66, 3.92, 4.51 },
      { 30, 1.31, 1.70, 2.04, 2.75, 3.03, 3.65, 3.90, 4.48 },
      { 35, 1.31, 1.69, 2.03, 2.72, 3.00, 3.59, 3.84, 4.39 },
      { 40, 1.30, 1.68, 2.02, 2.70, 2.97, 3.55, 3.79, 4.32 },
      { 45, 1.30, 1.68, 2.01, 2.69, 2.95, 3.52, 3.75, 4.27 },
      { 50, 1.30, 1.68, 2.01, 2.68, 2.94, 3.50, 3.72, 4.23 },
      { 55, 1.30, 1.67, 2.00, 2.67, 2.92, 3.48, 3.70, 4.20 },
      { 60, 1.30, 1.67, 2.00, 2.66, 2.91, 3.46, 3.68, 4.17 },
      { 65, 1.29, 1.67, 2.00, 2.65, 2.91, 3.45, 3.66, 4.15 },
      { 70, 1.29, 1.67, 1.99, 2.65, 2.90, 3.43, 3.65, 4.13 },
      { 75, 1.29, 1.67, 1.99, 2.64, 2.89, 3.42, 3.64, 4.11 },
      { 80, 1.29, 1.66, 1.99, 2.64, 2.89, 3.42, 3.63, 4.10 },
      { 85, 1.29, 1.66, 1.99, 2.63, 2.88, 3.41, 3.62, 4.08 },
      { 90, 1.29, 1.66, 1.99, 2.63, 2.88, 3.40, 3.61, 4.07 },
      { 95, 1.29, 1.66, 1.99, 2.63, 2.87, 3.40, 3.60, 4.06 },
      { 100, 1.29, 1.66, 1.98, 2.63, 2.87, 3.39, 3.60, 4.05 },
      { 200, 1.29, 1.65, 1.97, 2.60, 2.84, 3.34, 3.54, 3.97 },
      { 500, 1.28, 1.65, 1.96, 2.59, 2.82, 3.31, 3.50, 3.92 },
      { 1000, 1.28, 1.65, 1.96, 2.58, 2.81, 3.30, 3.49, 3.91 },
      { -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0 }
};

#define T_COLUMN_START 1
#define T_COLUMN_END 10

static double t_column_p[9] = 
      { -1.0, 0.1, 0.05, 0.025, 0.005, 0.0025, 0.0005, 0.00025, 0.00005 };

double paired_t_test(double * dist_x, double * dist_y, unsigned dist_size) {
    double * diff;
    unsigned i;
    double tot = 0.0, mean;
    double sd = 0.0;
    double t;

    diff = util_malloc_or_die(sizeof(*diff) * dist_size);
    for (i = 0; i < dist_size; i++) {
        diff[i] = dist_x[i] - dist_y[i];
        tot += diff[i];
    }
    mean = tot / dist_size;
    for (i = 0; i < dist_size; i++) {
        double d;
        d = diff[i] - mean;
        sd += d * d;
    }
    sd /= (dist_size - 1);
    sd = sqrt(sd);
    t = mean * sqrt(dist_size) / sd;
    return t;
}

double paired_t_test_p(double * dist_x, double * dist_y, unsigned dist_size,
  void * data) {
    double t = paired_t_test(dist_x, dist_y, dist_size);
    double df = dist_size - 1;
    unsigned d;
    unsigned s;
    for (d = 0; t_significance_table[d][0] > 0.0 
      && t_significance_table[d][0] < df; d++) 
        ;
    if (t_significance_table[d][0] < 0.0) {
        d--;
    }
    for (s = T_COLUMN_END; s > T_COLUMN_START; s--) {
        if (t_significance_table[d][s - 1] < t) {
            return t_column_p[s - 1];
        }
    }
    return 1.0;
}

#ifdef T_MAIN

#include <stdio.h>
#include <stdlib.h>

#define MAX_DIST_SIZE 1000
#define LINE_BUF_SIZE 1024

int main(void) {
    char line_buf[LINE_BUF_SIZE];
    double dist_x[MAX_DIST_SIZE];
    double dist_y[MAX_DIST_SIZE];
    unsigned d;
    double t, p;
    for (d = 0; d < MAX_DIST_SIZE && fgets(line_buf, LINE_BUF_SIZE, 
          stdin) != NULL; d++) {
        if (!sscanf(line_buf, "%lf %lf", &dist_x[d], &dist_y[d])) {
            fprintf(stderr, "Error on line %d of input\n", d + 1);
            return 1;
        }
    }
    t = paired_t_test(dist_x, dist_y, d);
    p = paired_t_test_p(dist_x, dist_y, d, NULL);
    fprintf(stdout, "%lf %lf\n", t, p);
    return 0;
}

#endif /* T_MAIN */
