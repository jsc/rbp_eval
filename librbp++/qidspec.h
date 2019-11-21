#ifndef RBPCC_QIDSPEC_H
#define RBPCC_QIDSPEC_H

/* Specify a set of queries.  Currently we only support a full range
 * of numerical queries. */

#include <string>
#include <stdexcept>
#include <sstream>
#include <cstdlib>

namespace rbp {

class QidSpecException : public std::invalid_argument {
    public:
        QidSpecException(const char * message) : invalid_argument(message) {}
};

class QidSpec {

    public:
    class Iterator {
        private: 
            QidSpec & spec_;
            int qid_;

        public:
            Iterator(QidSpec & spec, int qid) : spec_(spec),
                qid_(qid){ }
            Iterator operator++(int) { qid_++; return *this; }
            std::string operator*() { 
                std::ostringstream oss;
                oss << qid_;
                return oss.str();
            }
            bool operator==(const Iterator &it) {
                return it.qid_ == qid_;
            }
            bool operator!=(const Iterator &it) {
                return it.qid_ != qid_;
            }
    };

    private:
        /* PNAMBC */
        int first_;
        int last_;

    public:
        QidSpec(const std::string &spec);
        bool contains(const std::string &qid) {
            int qid_i = std::atoi(qid.c_str());
            if (qid_i >= first_ && qid_i <= last_)
                return true;
            else
                return false;
        }

        Iterator begin() { return Iterator(*this, first_); }
        Iterator end() { return Iterator(*this, last_ + 1); }
};

}

#endif /* RBPCC_QIDSPEC_H */
