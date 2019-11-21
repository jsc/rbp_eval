#ifndef RBPCC_RUN_H
#define RBPCC_RUN_H

#include <vector>
#include <map>
#include <stdexcept>
#include <string>
#include <stdexcept>
#include <iostream>

namespace rbp {


/*
 *  A document output by a system in a run.
 */
class RunDoc {

    public:
        class ParseException : public std::invalid_argument {
            public:
                ParseException(std::string message) 
                    : invalid_argument(message) { }
        };

    private:
        std::string docid_;
        unsigned occur_;    /* order of occurence in run */
        unsigned rank_;     /* stated rank in run */
        double score_;      /* score provided in run */

    public:
        RunDoc(std::string docid, unsigned occur, unsigned rank,
          double score) : docid_(docid), occur_(occur), rank_(rank),
          score_(score) { }

        const static int max_run_line_len_ = 1024;

        /* Parse a RunDoc line from a run file stream. */
        static bool parse_run_line(RunDoc & rd, std::string & topicid_out,
          std::string & runid_out,
          std::istream & in, unsigned occur=0) throw (ParseException);

        RunDoc() { }

        void set_occur(unsigned occur) { occur_ = occur; }

        const std::string &get_docid() const { return docid_; }
        unsigned get_occur() const { return occur_; }
        unsigned get_rank() const { return rank_; }
        double get_score() const { return score_; }
};

/*
 *  A system's run for a topic.
 */
class Run {
    public:
        typedef std::vector<RunDoc> DocListType;
        typedef DocListType::iterator Iterator;
        typedef DocListType::const_iterator ConstIterator;

    private:
        DocListType ranking_;
        std::string topicid_;

    public:
        Run(std::string topicid="") : topicid_(topicid) { }
        void append(RunDoc & rd) {
            ranking_.push_back(rd);
        }
        void set_topicid(std::string topicid) { topicid_ = topicid; }
        void clear() { topicid_ = ""; ranking_.clear(); }
        Iterator begin() { return ranking_.begin(); }
        Iterator end() { return ranking_.end(); }
        ConstIterator begin() const { return ranking_.begin(); }
        ConstIterator end() const { return ranking_.end(); }

        const std::string & get_topicid() const { return topicid_; }
};

/*
 *  A system, that is, a set of runs for a set of topics.
 */
class System {
    public:
        typedef std::string TopicIdType;
        typedef std::map<TopicIdType, Run> RunMapType;
        typedef RunMapType::iterator Iterator;
        typedef RunMapType::const_iterator ConstIterator;

    private:
        RunMapType runs_;
        std::string sysid_;
        bool parseErrorDesc;

    public:
        System(std::string sysid="") : sysid_(sysid) { } 
        void set_error(bool desc) { parseErrorDesc = desc; }
        void set_sysid(std::string sysid) { sysid_ = sysid; }
        void add(std::string topicid, Run run) {
            runs_[topicid] = run;
        }
        Iterator begin() { return runs_.begin(); }
        Iterator end() { return runs_.end(); }
        ConstIterator begin() const { return runs_.begin(); }
        ConstIterator end() const { return runs_.end(); }

        const std::string & get_sysid() const { return sysid_; }
        bool get_error(void) { return parseErrorDesc; }

        static void parse_run_file(System & sys, std::istream & in) throw
            (RunDoc::ParseException); 

        void dump(std::ostream & out);
};

};

#endif /* RBPCC_RUN_H */
