#ifndef RBPCC_SYSRANK_TOPIC_HASH_H
#define RBPCC_SYSRANK_TOPIC_HASH_H

/* XXX this code very likely doesn't work */

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

class SysRankList {
    public:
        typedef std::vector<SysRankItem> List;
        typedef List::iterator Iterator;

    private:
        List sysranking_;

    public:
        SysRankList() { }
        void append(SysRankItem & sri) {
            sysranking_.push_back(sri);
        }
        unsigned size() { return sysranking_.size(); }
        Iterator begin() { return sysranking_.begin(); } 
        Iterator end() { return sysranking_.end(); }
        bool empty() { return sysranking_.size() == 0; }
};

// Hash implementation
class Docs {
    public:
        typedef std::string DocidType;
        typedef Hash<DocidType, SysRankList>::Type Map;
        typedef Map::iterator Iterator;

    private:
        Map docsysmap_;

    public:
        Docs() { }
        void add(const DocidType & docid, SysRankList & list);
        SysRankList get(const DocidType & docid) const;
        void set(const DocidType &docid, SysRankList & newlist);
        unsigned size() const { return docsysmap_.size(); }
        Iterator begin() { return docsysmap_.begin(); }
        Iterator end() { return docsysmap_.end(); }
        bool empty() { return docsysmap_.size() == 0; }
};

class SysRank {
    public:
        class ParseException : public std::invalid_argument {
            public:
                ParseException(const char * message)
                    : invalid_argument(message) { }
        };
        typedef unsigned TopicidType;
        typedef Hash<TopicidType, Docs>::Type Map;
        typedef Map::iterator Iterator;

    private:
        Map topicdocmap_;

    public:
        SysRank() { }
        void add(const TopicidType & topicid, Docs & docmap);
        Docs * get(const TopicidType topicid);
        void set(const TopicidType topicid, Docs & newdocs);
        unsigned size() const { return topicdocmap_.size(); }
        Iterator begin() { return topicdocmap_.begin(); }
        Iterator end() { return topicdocmap_.end(); }

        SysRank fill(SysList sl) throw (ParseException);
        void print_info(SysRank info);
        void print_info_topic(SysRank info, unsigned topicid);
        void print_info_doc(SysRank info, std::string docid);
};

};


#endif /* RBPCC_SYSRANK_TOPIC_HASH_H */
