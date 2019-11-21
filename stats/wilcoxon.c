#include "util.h"
#include "wilcoxon.h"
#include <math.h>
#include <string.h>

#include <stdio.h>

#define NUM_OUTCOMES(d_sz) (((d_sz) * ((d_sz) + 1)) / 2 + 1)

static void generate_wilcoxon_outcomes_next(unsigned curr_dist_size,
  unsigned long long * from, unsigned long long * to) {
    unsigned next_size = curr_dist_size + 1;
    unsigned i;
    unsigned curr_num_outcomes;
    unsigned next_num_outcomes;

    curr_num_outcomes = NUM_OUTCOMES(curr_dist_size);
    next_num_outcomes = NUM_OUTCOMES(next_size);
    for (i = 0; i < curr_num_outcomes; i++) {
        to[i] = from[i];
    }
    for (i = curr_num_outcomes; i < next_num_outcomes; i++) {
        to[i] = 0;
    }
    for (i = 0; i < curr_num_outcomes; i++) {
        to[i + next_size] += from[i];
    }

}

unsigned generate_wilcoxon_outcomes(unsigned dist_size,
  unsigned long long * outcomes_table) {
    unsigned num_outcomes;
    unsigned long long * out_tmp;
    unsigned d;

    num_outcomes = NUM_OUTCOMES(dist_size);
    out_tmp = util_malloc_or_die(sizeof(*out_tmp) * num_outcomes);
    outcomes_table[0] = 1;
    outcomes_table[1] = 1;
    for (d = 1; d < dist_size; d++) {
        if (d % 2 == 1)
            generate_wilcoxon_outcomes_next(d, outcomes_table,
              out_tmp);
        else
            generate_wilcoxon_outcomes_next(d, out_tmp,
              outcomes_table);
    }
    if (dist_size % 2 == 0) {
        memcpy(outcomes_table, out_tmp, sizeof(out_tmp[0])
          * num_outcomes);
    }
    free(out_tmp);
    return num_outcomes;
}

int abs_cmp(const void * va, const void * vb) {
    double a = fabs(* (double *) va);
    double b = fabs(* (double *) vb);
    if (a > b) {
        return 1;
    } else if (b > a) {
        return -1;
    } else {
        return 0;
    }
}

#define MAX_CACHED_DIST_SIZE 51

double paired_wilcoxon_test_p(double * dist_x, double * dist_y,
  unsigned dist_size, void * data) {
    double * diff;
    unsigned i;
    unsigned diff_dist_size = 0;
    unsigned diff_num_outcomes;
    unsigned long long * outcomes_table = NULL;
    double wlc_pve;
    double critical_region_size;
    /* double rem; */
    static unsigned long long * outcomes_tables[MAX_CACHED_DIST_SIZE];
    static int outcomes_tables_inited = 0;

    if (!outcomes_tables_inited) {
        for (i = 0; i < MAX_CACHED_DIST_SIZE; i++) {
            outcomes_tables[i] = NULL;
        }
        outcomes_tables_inited = 1;
    }

    diff = util_malloc_or_die(sizeof(*diff) * dist_size);
    for (i = 0; i < dist_size; i++) {
        if (dist_x[i] != dist_y[i]) {
            diff[diff_dist_size++] = dist_x[i] - dist_y[i];
        }
    }
    if (diff_dist_size == 0) {
        free(diff);
        return 1.0;
    }
    diff_num_outcomes = NUM_OUTCOMES(diff_dist_size);
    if (diff_dist_size < MAX_CACHED_DIST_SIZE) {
        outcomes_table = outcomes_tables[diff_dist_size];
    }
    if (outcomes_table == NULL)
        outcomes_table = util_malloc_or_die(sizeof(*outcomes_table) 
          * diff_num_outcomes);
    generate_wilcoxon_outcomes(diff_dist_size, outcomes_table);
    qsort(diff, diff_dist_size, sizeof(*diff), abs_cmp);
    wlc_pve = 0.0;
    for (i = 0; i < diff_dist_size; ) {
        unsigned j;
        double abs_diff = fabs(diff[i]);
        double mean_rank;
        for (j = i + 1; j < diff_dist_size && fabs(diff[j]) == abs_diff;
          j++)
            ;
        mean_rank = (double)(i + j + 1) / 2.0;
        for ( ; i < j; i++) {
            if (diff[i] > 0.0) {
                wlc_pve += mean_rank;
            }
        }
    }
    critical_region_size = 0.0;
    /* The description of the Wilcoxon test are unclear on what should
     * be done in the case of half-values (that can occur due to tied
     * ranks).  We round up to the next whole value.  The commented-out
     * code took half of the previous value. */
    /*rem = fmod(wlc_pve, 1.0);
    critical_region_size = rem * outcomes_table[(unsigned) floor(wlc_pve)];*/
    for (i = (unsigned) ceil(wlc_pve); i < diff_num_outcomes; i++) {
        critical_region_size += outcomes_table[i];
    }
    free(diff);
    if (diff_dist_size < MAX_CACHED_DIST_SIZE) {
        outcomes_tables[diff_dist_size] = outcomes_table;
    } else {
        free(outcomes_table);
    }
    return critical_region_size / pow(2.0, diff_dist_size);
}

#ifdef WILCOXON_MAIN

#include <stdio.h>
#include <stdlib.h>

#define MAX_DIST_SIZE 1000
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
    p = paired_wilcoxon_test_p(dist_x, dist_y, d, NULL);
    fprintf(stdout, "%lf\n", p);
    return 0;
}

#endif /* WILCOXON_MAIN */
