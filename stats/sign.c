#include "binomial.h"

double paired_sign_test_p(double * dist_x, double * dist_y,
  unsigned dist_size, void * data) {
    unsigned pairs_greater;
    unsigned pairs_not_equal;
    unsigned i;
    double p;

    pairs_greater = 0;
    pairs_not_equal = dist_size;
    for (i = 0; i < dist_size; i++) {
        if (dist_x[i] > dist_y[i])
            pairs_greater++;
        else if (dist_x[i] == dist_y[i])
            pairs_not_equal--;
    }
    p = cumulative_binomial_p(pairs_not_equal, pairs_greater, 0.5);
    return p;
}

#ifdef SIGN_MAIN

#include <stdio.h>
#include <stdlib.h>

#define MAX_DIST_SIZE 10000
#define LINE_BUF_SIZE 1024

int main(void) {
    char line_buf[LINE_BUF_SIZE];
    double dist_x[MAX_DIST_SIZE];
    double dist_y[MAX_DIST_SIZE];
    unsigned d;
    double p;

    for (d = 0; d < MAX_DIST_SIZE && fgets(line_buf, LINE_BUF_SIZE, 
          stdin) != NULL; d++) {
        if (!sscanf(line_buf, "%lf %lf", &dist_x[d], &dist_y[d])) {
            fprintf(stderr, "Error on line %d of input\n", d + 1);
            return 1;
        }
    }
    p = paired_sign_test_p(dist_x, dist_y, d, NULL);
    fprintf(stdout, "%lf\n", p);
    return 0;
}

#endif /* SIGN_MAIN */
