#ifndef RBPCC_RBP_H
#define RBPCC_RBP_H

#include <cassert>
#include <cmath>
#include <vector>
#include <map>
#include <stdlib.h>
#include <iostream>

#include "qrels.h"
#include "run.h"

extern "C" {
#include <librbp/qdocs.h>
}

extern "C" {
    void librbpxx_is_present(void);
}

#define neg_epsilon -1.0e-10

namespace rbp {

/*
 * RBP calculation routines.
 */

/*
 *  RBP score, as a base plus residual.
 */
class RbpScore {
    private:
        double base_;
        double residual_;
        double nrel_;

    public:
        RbpScore(double base=0.0, double residual=1.0, double nrel=0.0) :
            base_(base), residual_(residual), nrel_(nrel) { }
        void incr(double rel, double wgt) {
            base_ += rel * wgt;
            residual_ -= wgt;
            nrel_ += rel;
            assert(base_ <= 1.0);
            if (residual_ < neg_epsilon) {
                std::cerr << "ERROR\n"
                          << "base = " << base_ << " residual = " << residual_ 
                          << " nrel = " << nrel_ << "\n";
            }
            // To deal with the floating-point accuracy problem
            assert(residual_ >= neg_epsilon);
        }
        double get_base() { return base_; }
        double get_residual() { return residual_; }
        double get_nrel() { return nrel_; }
};

/* List of Rbp scores, for different topics */
class RbpScoreList {
    public:
        typedef Qrelset::TopicidType TopicidType;
        typedef std::map<TopicidType, RbpScore> RbpMapType;
        typedef RbpMapType::iterator Iterator;

    private:
        RbpMapType rbpscores_;

    public:
        RbpScoreList() { }
        void add(TopicidType topicid, RbpScore score) {
            rbpscores_[topicid] = score;
        }
        Iterator begin() { return rbpscores_.begin(); }
        Iterator end() { return rbpscores_.end(); }
};

class RbpScorer {
    private:
        double persist_;

    protected:
        /* filter list of rels just before performing the 
         * calculation.  If this method returns false, then
         * the original list of rels is used unchanged; otherwise,
         * the filtered in $out$ is used.  This method gets
         * called at the start of score(rels). */
        virtual bool filter_rels(const std::vector<double> &in,
          std::vector<double> &out) { return false; }
        virtual RbpScore adjust_score(RbpScore score) { return score; }

    public:
        virtual ~RbpScorer() {};
        /* RBP weight at depth $rank$ (counting from 0). */
        double weight(unsigned rank) {
            double wgt = (1 - persist_) * pow(persist_, rank);
            assert(wgt <= 1.0);
            assert(wgt >= 0.0);
            return wgt;
        }
        /* residual remaining after judging $depth$ documents (counting
         * from 1). */
        double residual(unsigned depth) {
            return pow(persist_, depth);
        }

    public:
        RbpScorer(double persist) : persist_(persist) { }
        RbpScore score(const std::vector<double> &rels);
        RbpScore score(const Qrels &qrels, 
          const std::vector<std::string> &docs);
        RbpScore score(const Qrels &qrels, qdocs_t * qdocs);
        RbpScore score(const Qrels &qrels, const Run &run);

};

class InducedRbpScorer : public RbpScorer {
    public:
        /* length of the filtered and unfiltered ranking lists for
         * the most recently scored run.  Note that this makes the
         * class non-reentrant. */
        int last_filtered_length;
        int last_unfiltered_length;
    private:
        /* for docs judged but not retrieved, smear the residual
         * across the remained ranks.  To do this, we need to 
         * know the total number of documents judged, and how
         * many of them were judged relevant. */
        bool smear_residual_;
        unsigned judgments_;
        unsigned num_rel_;
    public:
        InducedRbpScorer(double persist, bool smear_residual=false,
          unsigned judgments=0, unsigned num_rel=0) 
            : RbpScorer(persist), smear_residual_(smear_residual),
              judgments_(judgments), num_rel_(num_rel) { }
        virtual bool filter_rels(const std::vector<double> &in,
          std::vector<double> &out);
        virtual RbpScore adjust_score(RbpScore score);
};


}; /* end namespace */

#endif /* RBPCC_RBP_H */
