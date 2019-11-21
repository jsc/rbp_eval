#ifndef RBPCC_QRELS_H
#define RBPCC_QRELS_H

#include <string>
#include <map>
#include <stdexcept>

#include "hash.h"

namespace rbp {

/*
 *  Relevance judgments, for an individual query.
 */
class Qrels {
    public:
        typedef Hash<std::string, double>::Type Map;
        typedef Map::iterator Iterator;
        //typedef std::map<std::string, double> Map;

    private:
        Map qrels_;

    public:
        Qrels() { }
        void add(const std::string & docid, double rel);
        double get(const std::string & docid) const;
        unsigned size() const { return qrels_.size(); }
        template<class Out> Out get_docs(Out begin);
        Iterator begin() { return qrels_.begin(); }
        Iterator end() { return qrels_.end(); }
        bool empty() { return qrels_.size() == 0; }
        void clear() { qrels_.clear(); }
};

template<class Out> Out Qrels::get_docs(Out begin) {
    Qrels::Map::iterator mit;
    for (mit = qrels_.begin(); mit != qrels_.end(); mit++) {
        *begin = mit->first;
        begin++;
    }
    return begin;
};

class Qrelset {
    public:
        typedef std::string TopicidType;
        typedef Hash<TopicidType, Qrels>::Type Map;
        typedef Map::iterator Iterator;

        class ParseException : public std::invalid_argument {
            public:
                ParseException(std::string message) 
                    : invalid_argument(message) { }
        };

    private:
        Map list_;

    public:
        Qrelset() { }
        void add(const TopicidType & topicid, Qrels & qrels);
        Qrels & get(const TopicidType & topicid) { 
            return list_[topicid]; } 
        void set(const TopicidType topicid, Qrels & newqrels);
        unsigned size() const { return list_.size(); }
        Iterator begin() { return list_.begin(); }
        Iterator end() { return list_.end(); }

        const static int max_qrel_line_len_ = 1024;

        static void parse_qrels_file(Qrelset & qrels, std::istream & in)
            throw (Qrelset::ParseException);
};

}; /* end namespace */

#endif /* RBPCC_QRELS_H */
