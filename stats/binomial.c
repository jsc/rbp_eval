#include <assert.h>
#include <math.h>

/*
 *  Calculate probability of a given outcome of a binomial distribution.
 *
 *  Takes three inputs:
 *
 *    @param trials      the number of trials to perform
 *    @param successes   look for this number or more succesful trials
 *    @param probability the probability of success for each trial.
 *
 *  Long doubles are used internally because simple doubles start overflowing
 *  around the 10,000 trial mark (depending on how many successes
 *  you're looking for).
 */
double cumulative_binomial_p(unsigned trials,
  unsigned successes, double probability) {
    long double x, r;
    unsigned i;

    assert(successes <= trials);
    assert(probability >= 0.0);
    assert(probability <= 1.0);

    r = (1 - probability) / probability;
    x = 1.0;
    for (i = 1; i <= trials - successes; i++) {
        x = x * r * (successes + i) / (float)(trials - successes + 1 - i) + 1.0;
    }
    return (double) exp(log(x) + (trials * log(probability)));
}

#ifdef BINOMIAL_MAIN

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char ** argv) {
    unsigned trials;
    unsigned successes;
    double probability;
    double p;

    if (argc != 4) {
        fprintf(stderr, "Usage: %s <trials> <successes> <probability>\n",
          argv[0]);
        return 1;
    }
    trials = atoi(argv[1]);
    successes = atoi(argv[2]);
    probability = atof(argv[3]);
    p = cumulative_binomial_p(trials, successes, probability);
    fprintf(stderr, "%lf\n", p);
    return 0;
}

#endif /* BINOMIAL_MAIN */
