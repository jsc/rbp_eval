#include "qrels.h"
#include <sstream>

extern "C" {
#include <ctype.h>
}

using namespace rbp;

void Qrels::add(const std::string &docid, double rel) {
    std::string docid_cpy = docid;
    for (int i = 0; i < docid_cpy.length(); i++)
        docid_cpy[i] = tolower(docid_cpy[i]);
    qrels_[docid_cpy] = rel;
}

double Qrels::get(const std::string &docid) const {
    std::string docid_cpy = docid;
    for (int i = 0; i < docid_cpy.length(); i++)
        docid_cpy[i] = tolower(docid_cpy[i]);
    Qrels::Map::const_iterator cit;
    cit = qrels_.find(docid_cpy);
    if (cit == qrels_.end()) {
        return -1.0;
    } else {
        return cit->second;
    }
}

void Qrelset::add(const TopicidType & topicid, Qrels & qrels) {
    list_[topicid] = qrels;
}

/*Qrels & Qrelset::get(const TopicidType & topicid) {
    Qrelset::Map::const_iterator cit;
    Qrels emptyqrels;
    cit = list_.find(topicid);
    if (cit == list_.end()) {
        return emptyqrels;
    } else {
        return cit->second;
    }
}*/

void Qrelset::set(const TopicidType topicid, Qrels & newqrels) {
    Qrelset::Map::iterator Iter;
    Iter = list_.find(topicid);
    Iter->second = newqrels;
}

void Qrelset::parse_qrels_file(Qrelset & qrelset, std::istream & sin)
    throw (Qrelset::ParseException) {

    bool inputp = true;
    std::string last_topicid = "";
    bool first_line = true;
    char buf[max_qrel_line_len_ + 1];
    Qrels qrels;

    while (inputp) {
        std::string topicid, dummy, docid;
        int rel;
        sin.getline(buf, max_qrel_line_len_ + 1);
        if (sin.gcount() > max_qrel_line_len_) {
            throw ParseException("Line longer than max qrel line length");
        } else if (sin.eof()) {
            inputp = false;
        } else if (!sin.good()) {
            throw ParseException("Error on input stream");
        }

        if (inputp) {
            std::istringstream ss(buf);

            ss >> topicid >> dummy >> docid >> rel;
            if (ss.fail() || !ss.eof()) {
                throw ParseException("Invalid qrel line: '" + std::string(buf) 
                  + "'");
            }
        }
        if (!inputp || topicid != last_topicid) {
            if (!first_line) {
                qrelset.add(last_topicid, qrels);
                qrels.clear();
            }
            first_line = false;
            last_topicid = topicid;
        }
        if (inputp) {
            qrels.add(docid, rel);
        }
    }
}
