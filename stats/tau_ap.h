/* 
 *  Efficient implementation of tau_ap.
 */

#ifndef TAU_AP_H
#define TAU_AP_H

#include <cstddef> /* for NULL */

namespace rbp {

class CtBNode {
    private:
        int val_;
        int st_count_;
        CtBNode * parent_, * left_, * right_;

    public:
        CtBNode(CtBNode * parent, int val) : val_(val), st_count_(1), 
            parent_(parent), left_(NULL), right_(NULL) {};
        ~CtBNode();
        void add_descendant(int val);
        int get_st_count() const { return st_count_; }
};

};

#endif /* TAU_AP_H */
