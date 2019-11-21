#include "qidspec.h"
#include <cstdlib>

using namespace rbp;

QidSpec::QidSpec(const std::string &spec) {
    std::string::size_type dash = spec.find("-");
    if (dash == std::string::npos) {
        throw QidSpecException("no dash in qid spec");
    }
    first_ = std::atoi(spec.c_str());
    last_ = std::atoi(spec.c_str() + dash + 1);
    if (last_ < first_) {
        throw QidSpecException("last later than first in qid spec");
    }
}
