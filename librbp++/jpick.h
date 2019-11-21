#ifndef RBPCC_JPICK_H
#define RBPCC_JPICK_H

#include "judgment.h"

namespace rbp {

/*
 * Pick documents for judging.
 */
class JudgmentPicker {
    public:
        virtual Judgment pick() = 0;
        virtual bool finished() = 0;
        virtual ~JudgmentPicker() {};
};

};

#endif /* RBPCC_JPICK_H */
