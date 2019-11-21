#ifndef RBPCC_JUDGMENT_H
#define RBPCC_JUDGMENT_H 

#include <string>

namespace rbp {

class Judgment {
    private:
        std::string qid_;
        std::string docid_;
        double rel_;

    public:
        Judgment(std::string qid, std::string docid, double rel) :
            qid_(qid), docid_(docid), rel_(rel) {}
        const std::string &get_qid() const { return qid_; }
        const std::string &get_docid() const { return docid_; }
        double get_rel() const { return rel_; }
};

}; /* end namespace */

#endif /* RBPCC_JUDGMENT_H */
