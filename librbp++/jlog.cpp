#include "jlog.h"
#include <stdexcept>
#include <sstream>

using namespace rbp;

void JudgmentLog::load(std::istream &in) {
    std::string line;
    while (getline(in, line)) {
        std::istringstream ss(line);
        std::string qid, docid;
        double rel;
        if (!(ss >> qid) || !(ss >> docid ) || !(ss >> rel)) {
            throw std::invalid_argument("Invalid jlog line: " + line);
        }
        Judgment judgment(qid, docid, rel);
        judgments_.push_back(judgment);
    }
}
