#ifndef RBPCC_RUNGROUPS_H
#define RBPCC_RUNGROUPS_H

/*
 *  Group runs into groups.
 */

#include <string>
#include <set>
#include <map>
#include <istream>
#include <stdexcept>

#include "hash.h"

namespace rbp {

class RunGroupException : public std::invalid_argument {
    public:
        RunGroupException(const char * message) : invalid_argument(message) {}
};

class RunGroup {

    public:
        typedef std::set<std::string> RunidList;

    private:
        std::string group_name_;
        RunidList runids_;

    public:
        RunGroup() { }
        RunGroup(const std::string &name) : group_name_(name) { }
        RunGroup(const RunGroup &g) : group_name_(g.get_group_name()) {
            runids_ = g.runids_;
        }
        RunGroup & operator=(const RunGroup &g) {
            group_name_ = g.group_name_;
            runids_ = g.runids_;
            return *this;
        }
        void add_runid(const std::string &runid) {
            runids_.insert(runid);
        }
        bool contains(const std::string &runid) const {
            return runids_.find(runid) != runids_.end();
        }
        const std::string get_group_name() const { return group_name_; }
        const RunidList & get_runids() const { return runids_; }
};

class RunGroups {
    public:
        typedef std::map<std::string, RunGroup> RunGroupMap;
        typedef RunGroupMap::const_iterator ConstIterator;

    private:
        /* typedef Hash<std::string &, RunGroup &> RunidMap; */
        RunGroupMap groups_;

    public:
        RunGroups() { }
        void add_run_group(const RunGroup & run_group) {
            groups_[run_group.get_group_name()] = run_group;
        }
        // Parse from an input stream.
        //
        // Each input line defines a group, using the format:
        //   <group_name>: <runid> ...
        void parse(std::istream &in);

        const RunGroupMap & get_groups() const { return groups_; }
        const ConstIterator begin() const { return groups_.begin(); }
        const ConstIterator end() const { return groups_.end(); }
};

};

#endif /* RBPCC_RUNGROUPS_H */
