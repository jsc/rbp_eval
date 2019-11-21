#include <assert.h>
#include <stdio.h>

double kendall_tau(double * dat1, double * dat2, unsigned dat_size) {
    unsigned i, j;
    int concordant, discordant;
    double tau;

    if (dat_size < 2) {
        /* doesn't really make sense, but... */
        return 1.0;
    }
    concordant = discordant = 0;
    for (i = 0; i < dat_size; i++) {
        for (j = i + 1; j < dat_size; j++) {
            if (dat1[i] > dat1[j]) {
                if (dat2[i] > dat2[j]) {
                    concordant += 1;
                } else {
                    discordant += 1;
                }
            } else if (dat1[i] < dat1[j]) {
                if (dat2[i] < dat2[j]) {
                    concordant += 1;
                } else {
                    discordant += 1;
                }
            } else {
                if (dat2[i] == dat2[j]) {
                    concordant += 1;
                } else {
                    discordant += 1;
                }
            }
        }
    }
    tau = (float) (concordant - discordant) / ((dat_size * (dat_size - 1) / 2));
    assert(tau <= 1.0);
    assert(tau >= -1.0);
    return tau;
}

#ifdef KENDALL_MAIN

#include <stdio.h>
#include <stdlib.h>

#define MAX_DIST_SIZE 1000
#define LINE_BUF_SIZE 1024

int main(void) {
    char line_buf[LINE_BUF_SIZE];
    double dat1[MAX_DIST_SIZE];
    double dat2[MAX_DIST_SIZE];
    unsigned d;
    double tau;

    for (d = 0; d < MAX_DIST_SIZE && fgets(line_buf, LINE_BUF_SIZE, 
          stdin) != NULL; d++) {
        if (!sscanf(line_buf, "%lf %lf", &dat1[d], &dat2[d])) {
            fprintf(stderr, "Error on line %d of input\n", d + 1);
            return 1;
        }
    }
    tau = kendall_tau(dat1, dat2, d);
    fprintf(stdout, "%lf\n", tau);
    return 0;
}

#endif /* KENDALL_MAIN */
