#include "rbp.h"
#include <cassert>

using namespace rbp;

extern "C" {
void librbpxx_is_present(void) {
}
}

RbpScore RbpScorer::score(const std::vector<double> &inrels) {
    std::vector<double> filtered;
    const std::vector<double> * the_rels;
    if (filter_rels(inrels, filtered))
        the_rels = &filtered;
    else
        the_rels = &inrels;

    RbpScore score(0.0, 1.0);
    for (unsigned i = 0; i < the_rels->size(); i++) {
        double wgt;
        double rel = (*the_rels)[i];
        if (rel > 1.0)
            rel = 1.0;
        if (rel >= 0.0) {
            wgt = weight(i);
            score.incr(rel, wgt);
        }
    }
    return adjust_score(score);
}

RbpScore RbpScorer::score(const Qrels &qrels,
  const std::vector<std::string> &docs) {
    /* convert list of docs into list of relevancies. */
    std::vector<double> rels;
    for (std::vector<std::string>::const_iterator cit = docs.begin();
      cit != docs.end(); cit++) {
        double rel = qrels.get(*cit);
        rels.push_back(rel);
    }
    return score(rels);
}

RbpScore RbpScorer::score(const Qrels &qrels, qdocs_t * qdocs) {
    doc_score_t * ds;
    unsigned num_scores, c;
    std::vector<std::string> rdocs;

    num_scores = qdocs_num_scores(qdocs);
    ds = qdocs_get_scores(qdocs, QDOCS_ORD_SCORE);
    for (c = 0; c < num_scores; c++) {
        rdocs.push_back(std::string(ds[c].docid));
    }
    return score(qrels, rdocs);
}

RbpScore RbpScorer::score(const Qrels &qrels, const Run &run) {
    /* convert list of RunDocs into list of relevancies. */
    std::vector<double> rels;
    for (Run::ConstIterator it = run.begin(); it != run.end(); it++) {
        double rel = qrels.get(it->get_docid());
        rels.push_back(rel);
    }
    return score(rels);
}

bool InducedRbpScorer::filter_rels(const std::vector<double> &in,
  std::vector<double> &out) {
    last_unfiltered_length = in.size();
    for (std::vector<double>::const_iterator cit = in.begin();
      cit != in.end(); cit++) {
        if (*cit >= 0.0)
            out.push_back(*cit);
    }
    last_filtered_length = out.size();
    return true;
}

RbpScore InducedRbpScorer::adjust_score(RbpScore score) {
    if (smear_residual_) {
        unsigned num_judged_in_run = last_filtered_length;
        unsigned num_rel_in_run = (unsigned) score.get_nrel();
        assert(num_judged_in_run <= judgments_);
        assert(num_rel_in_run <= num_rel_);
        unsigned extension_length = judgments_ - num_judged_in_run;
        unsigned num_rel_remain = num_rel_ - num_rel_in_run;
        if (extension_length > 0) {
            double extension_rel_prop
                = (double) num_rel_remain / extension_length;
            double extension_residual = residual(num_judged_in_run) -
                residual(judgments_);
            score = RbpScore(score.get_base() + extension_residual *
              extension_rel_prop, residual(judgments_), num_rel_);
        }
    }
    return score;
}
