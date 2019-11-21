#ifndef RBPCC_SYSRANK_TOPIC_H
#define RBPCC_SYSRANK_TOPIC_H

/*
 *  System rankings by topic id .
 */
#include "syslist.h"
#include <vector>
#include <map>
#include <stdexcept>
#include <string>
#include <stdexcept>
#include <algorithm>

#include "hash.h"

extern "C" {
#include <librbp/run.h>
}

namespace rbp {

class SysRankItem {
    public:

    private:
        std::string sysid_;
        unsigned rank_;

    public:
        SysRankItem(std::string sysid="") : sysid_(sysid) { }
        unsigned get_rank() { return rank_; }
        void set_rank(unsigned rank) { rank_ = rank; }
        std::string &get_sysid() { return sysid_; }
        void set_sysid(std::string sysid) { sysid_ = sysid; }
};

class DocItem {
    public:
        typedef std::vector<SysRankItem> SysRankList;
        typedef SysRankList::iterator Iterator;

    private:
        SysRankList sysranking_;
        std::string docid_;

    public:
        DocItem(std::string docid="") : docid_(docid) { }
        void append(SysRankItem & sri) {
            sysranking_.push_back(sri);
        }
        void insertinorder(SysRankItem & sri) {
            SysRankList::iterator Iter;
            Iter = findpos(sri.get_rank());
            sysranking_.insert(Iter, 1, sri);
        }
        Iterator findpos(unsigned rank) {
            SysRankList::iterator Iter;
            for (Iter = sysranking_.begin(); Iter != sysranking_.end(); 
              Iter++) {
                if ((*Iter).get_rank() > rank) {
                    return Iter++;
                }
            }
            return Iter;
        }

        std::string &get_docid() { return docid_; }
        void set_docid(std::string docid) { docid_ = docid; }
        void clear() { docid_ = ""; sysranking_.clear(); }
        unsigned size() { return sysranking_.size(); }
        Iterator begin() { return sysranking_.begin(); }
        Iterator end() { return sysranking_.end(); }
};

class TopicItem {
    public:
        typedef std::vector <DocItem> DocList;
        typedef DocList::iterator Iterator;

    private:
        DocList docranking_;
        unsigned topicid_;

    public:
        // Is there a topic id equal to 0?
        //TopicItem(unsigned topicid=0) : topicid_(topicid) { }
        TopicItem() { }
        void append(DocItem & di) {
            docranking_.push_back(di);
        }
        unsigned get_topicid() { return topicid_; }
        void set_topicid(unsigned topicid) { topicid_ = topicid; }
        // Is there a topic id equal to 0?
        //void clear() { topicid_ = 0; docranking_.clear(); }
        Iterator begin() { return docranking_.begin(); }
        Iterator end() { return docranking_.end(); }
};

class SysRank {
    public:
        class ParseException : public std::invalid_argument {
            public:
                ParseException(const char * message)
                    : invalid_argument(message) { }
        };
        typedef std::vector <TopicItem> TopicList;
        typedef TopicList::iterator Iterator;

    private:
        TopicList topicranking_;

    public:
        SysRank() { }
        void append(TopicItem & ti) {
            topicranking_.push_back(ti);
        }
        void insertinorder(TopicItem & ti) {
            TopicList::iterator Iter;
            Iter = findpos(ti.get_topicid());
            topicranking_.insert(Iter, 1, ti);
        }
        Iterator findpos(unsigned topicid) {
            TopicList::iterator Iter;
            for (Iter = topicranking_.begin(); Iter != topicranking_.end(); Iter++) {
                if ((*Iter).get_topicid() > topicid) {
                    return Iter++;
                }
            }
            return Iter;
        }

        void clear() { topicranking_.clear(); }
        Iterator begin() { return topicranking_.begin(); }
        Iterator end() { return topicranking_.end(); }

        // comparison function
        bool compare_order (TopicItem first, TopicItem second) {
            unsigned int i=0;
            if (first.get_topicid() < second.get_topicid()) return true;
            else return false;
        }

        //void sortlist() { std::sort(topicranking_.begin(), topicranking_.end(), compare_order); }
        SysRank fill(SysList sl) throw (ParseException);
        void print_info(SysRank info);
        void print_info_topic(SysRank info, unsigned topicid);
        void print_info_doc(SysRank info, std::string docid);
};

};


#endif /* RBPCC_SYSRANK_TOPIC_H */
