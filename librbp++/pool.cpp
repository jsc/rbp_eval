#include <cstdlib>
#include "pool.h"

using namespace rbp;

void Pool::add(qdocs_t * qd, int depth) {
    int d;
    doc_score_t * ds;
    if (depth < 0 || depth > qdocs_num_scores(qd)) {
        depth = qdocs_num_scores(qd);
    }
    ds = qdocs_get_scores(qd, QDOCS_ORD_SCORE);
    for (d = 0; d < depth; d++) {
        add(ds[d].docid);
    }
}

void Pool::get_docs(std::vector<std::string> &vec) {
    for (std::set<std::string>::iterator it = docs_.begin();
      it != docs_.end(); it++) {
        vec.push_back(*it);
    }
}

void PoolSet::add(run_t * run, int depth) {
    unsigned int num_qdocs = run_num_qdocs(run);
    for (unsigned q = 0; q < num_qdocs; q++) {
        qdocs_t * qd = run_get_qdocs_by_index(run, q);
        std::string qid = qdocs_qid(qd);
        map_[qid].add(qd, depth);
    }
}
