#include "bootstrap.h"
#include "util.h"

#include <stdlib.h>
#include <unistd.h>

#define BOOTSTRAP_NUM_TRIALS 1000

double paired_bootstrap_test_p(double * dist_x, double * dist_y,
  unsigned dist_size, void * data) {
    double * diff;
    double * dist_w;
    unsigned i;
    double mean_diff;
    double tot_diff = 0.0;
    unsigned asl_count = 0;

    diff = util_malloc_or_die(sizeof(*diff) * dist_size);
    for (i = 0; i < dist_size; i++) {
        diff[i] = dist_x[i] - dist_y[i];
        tot_diff += diff[i];
    }
    mean_diff = tot_diff / dist_size;
    dist_w = util_malloc_or_die(sizeof(*dist_w) * dist_size);
    for (i = 0; i < dist_size; i++) {
        dist_w[i] = diff[i] - mean_diff;
    }

    srandom(getpid());
    for (i = 0; i < BOOTSTRAP_NUM_TRIALS ; i++) {
        /* we're depending on the goodness of the random-number
         * generator... */
        double sample_tot = 0.0;
        unsigned c;
        for (c = 0; c < dist_size; c++) {
            unsigned choice = random() % dist_size;
            sample_tot += dist_w[choice];
        }
        /* XXX we count '>=' rather than just '>' because this
         * gives a reasonable handling if dist_x and dist_y are
         * identical (p = 1.0, rather than p = 0.0) */
        if (sample_tot / dist_size >= mean_diff) {
            asl_count++;
        }
    }
    return (double) asl_count / BOOTSTRAP_NUM_TRIALS;
}

#ifdef BOOTSTRAP_MAIN

#define MAX_DIST_SIZE 1000
#define LINE_BUF_SIZE 1024

#include <stdio.h>

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
    p = paired_bootstrap_test_p(dist_x, dist_y, d, NULL);
    fprintf(stdout, "%lf\n", p);
    return 0;
}

#endif /* BOOTSTRAP_MAIN */
