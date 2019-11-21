/* 
 *  Efficient implementation of tau_ap.
 *  
 *  The algorithm is as follows:
 *
 *  1. Input: objective ranking R, compared ranking S.
 *  2. Put S into same order as R.
 *  3. Set T to empty, c to 0
 *  4. For each element s in S:
 *  5.   c += count elements in T less than s
 *  6.   T << s
 *
 *  $c$ is therefore the count of concordant elements.  The
 *  complexity of the algorithm is $n$ * the complexity of
 *  lines 5. and 6.  Using a binary tree for T makes these
 *  operations O(log n)
 */

#include "tau_ap.h"

using namespace rbp;

void CtBNode::add_descendant(int val) {
    st_count_ += 1;
    if (val < val_) {
        if (left_ == NULL) {
            left_ = new CtBNode(this, val);
        } else {
            left_->add_descendant(val);
        }
    } else {
        if (right_ == NULL) {
            right_ = new CtBNode(this, val);
        } else {
            right_->add_descendant(val);
        }
    }
}

CtBNode::~CtBNode() {
    if (left_ != NULL)
        delete(left_);
    if (right_ != NULL)
        delete(right_);
}
