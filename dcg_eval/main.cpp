/*
 *  Implement the (normalised) Discounted Cumulative Gain metric.
 */

#include <iostream>
#include <algorithm>
#include <librbp++/rbp.h>

extern "C" {
#include <unistd.h>
#include <librbp/run.h>
#include <librbp/qrels.h>
};

const unsigned default_depth = 1000;
const double default_discounting_base = 2;

using namespace rbp;
using namespace std;

void usage_and_exit(std::ostream & out, const char * progname, int exit_code) {
    out << "USAGE: " << progname 
        << " [-R] [-b base] [-r depth] [-n|N] [-d|D] [-m|M] <qrels> <run>" 
        << std::endl;
    exit(exit_code);
}

#define ERR_BUF_LEN 1024

/* #define VERBOSE */

int main(int argc, char ** argv) {
    int ret;
    FILE * qrels_fp = NULL;
    FILE * run_fp = NULL;
    run_t * run = NULL;
    qrels_t * qrels = NULL;
    char err_buf[ERR_BUF_LEN];
    int optflag;
    double discounting_base = default_discounting_base;
    char * qrels_fname, * run_fname;
    const char * progname;
    unsigned q;
    unsigned depth = default_depth;
    double log_e_b;
    bool normalised = true;
    bool discounted = true;
    enum reltype_t reltype = RELTYPE_AUTO;
    double tot_score = 0.0;
    unsigned num_queries = 0;
    unsigned rank_adjust = 0;

    progname = argv[0];
    while ( (optflag = getopt(argc, argv, "b:Rr:hnNdDmM")) != -1) {
        switch (optflag) {
        case 'R':
            /* add 1 to rank before discounting.  This is the MS (aka
             * 'non-stupid') version of DCG. */
            rank_adjust = 1;
            break;
        case 'b':
            discounting_base = atof(optarg);
            break;
        case 'r':
            depth = atoi(optarg);
            break;
        case 'n':
            normalised = true;
            break;
        case 'N':
            normalised = false;
            break;
        case 'm':
            reltype = RELTYPE_FRACT;
            break;
        case 'M':
            reltype = RELTYPE_BINARY;
            break;
        case 'd':
            discounted = true;
            break;
        case 'D':
            discounted = false;
            break;
        case 'h':
            usage_and_exit(std::cout, progname, 0);
            break;
        }
    }

    if (argc - optind != 2) {
        usage_and_exit(std::cerr, argv[0], 1);
    }

    qrels_fname = argv[optind];
    run_fname = argv[optind + 1];

    qrels_fp = fopen(qrels_fname, "r");
    if (qrels_fp == NULL) {
        std::cerr << "Unable to open qrels file '" << qrels_fname 
            << "' for reading" << std::endl;
        usage_and_exit(std::cerr, progname, 1);
    }
    run_fp = fopen(run_fname, "r");
    if (run_fp == NULL) {
        std::cerr << "Unable to open run file '" << run_fname 
            << "' for reading" << std::endl;
        usage_and_exit(std::cerr, progname, 1);
    }

    run = load_run(run_fp, err_buf, ERR_BUF_LEN);
    fclose(run_fp);
    if (run == NULL) {
        std::cerr << "Error loading run file '" << run_fname << "': "
            << err_buf << std::endl;
        usage_and_exit(std::cerr, progname, 1);
    }

    qrels = load_qrels(qrels_fp, err_buf, ERR_BUF_LEN);
    fclose(qrels_fp);
    if (qrels == NULL) {
        std::cerr << "Error loading qrels file '" << qrels_fname << "': "
            << err_buf << std::endl;
        usage_and_exit(std::cerr, progname, 1);
    }

    qrels_set_reltype(qrels, reltype, 1.0);

    log_e_b = log(discounting_base);

    for (q = 0; q < run_num_qdocs(run); q++) {
        qdocs_t * qdocs;
        qid_qrels_t * qq;
        const char * qid;
        doc_score_t * ds;
        unsigned run_length;
        unsigned r;
        double score;

        qdocs = run_get_qdocs_by_index(run, q);
        qid = qdocs_qid(qdocs);
        qq = qrels_get_qid_qrels(qrels, qid);
        if (qq == NULL) {
            std::cerr << "No judgments for query " << qid 
                << "; ignoring" << std::endl;
            continue;
        }
        num_queries++;
        ds = qdocs_get_scores(qdocs, QDOCS_ORD_SCORE);
        run_length = qdocs_num_scores(qdocs);
        if (run_length > depth) {
            run_length = depth;
        }
        score = 0.0;
        for (r = 0; r < run_length; r++) {
            rel_t rel;

            rel = qid_qrels_get_rel(qq, ds[r].docid);
            if (rel > 0.0) {
                if (discounted && (r + 1 + rank_adjust >= discounting_base)) {
#ifdef VERBOSE
                    fprintf(stderr, "Discounting %lf at rank %u to ", rel, 
                      r + 1);
#endif /* VERBOSE */
                    rel /= (log(r + 1 + rank_adjust) / log_e_b);
#ifdef VERBOSE
                    fprintf(stderr, "%lf\n", rel);
#endif /* VERBOSE */
                }
                score += rel;
            }
        }
        if (normalised) {
            std::vector<rel_t> optimal_rel;
            qrels_iterator_t * qit;
            rel_t rel;
            double optimal_score = 0.0;
            unsigned r;

            qit = qid_qrels_get_iterator(qq);
            while ( (qrel_iter_next(qit, &rel)) != NULL) {
                if (rel > 0.0) {
                    optimal_rel.push_back(rel);
                }
            }
            qrel_iter_delete(&qit);
            sort(optimal_rel.begin(), optimal_rel.end());
            reverse(optimal_rel.begin(), optimal_rel.end());
            r = 0;
            for (std::vector<rel_t>::iterator it = optimal_rel.begin();
                it != optimal_rel.end(); it++, r++) {
                rel_t rel = *it;
                if (discounted && (r + 1 + rank_adjust >= discounting_base)) {
                    rel /= (log(r + 1 + rank_adjust) / log_e_b);
                }
                optimal_score += rel;
            }
#ifdef VERBOSE
            fprintf(stderr, "Normalising %lf to ", score);
#endif /* VERBOSE */
            if (optimal_score > 0) {
                score /= optimal_score;
            }
#ifdef VERBOSE
            fprintf(stderr, "%lf\n", score);
#endif /* VERBOSE */
        }
        tot_score += score;
        std::cout << qid << " " << score << std::endl;
    }
    std::cout << "all " << tot_score / num_queries << std::endl;

    return 0;
}
