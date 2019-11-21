#ifndef ARRAY_H
#define ARRAY_H

#include "util.h"

#define ARRAY_INIT_SPACE 5

/* Array management macros.
 * 
 * Using macros allows a measure of genericity, and also allows us
 * to use the word "genericity".
 */

#define ARRAY_TYPE_DECL(name, elem_type)                                      \
    typedef struct {                                                          \
        unsigned elem_count;                                                  \
        unsigned space;                                                       \
        elem_type * elems;                                                    \
    } name

#define ARRAY_INIT(a) {                                                       \
    (a).elem_count = 0;                                                       \
    (a).space = 0;                                                            \
    (a).elems = NULL;                                                         \
}


#define ARRAY_ADD(a, elem) {                                                  \
    if ((a).elem_count == (a).space) {                                        \
        if ((a).space == 0) {                                                 \
            (a).space = ARRAY_INIT_SPACE;                                     \
        } else {                                                              \
            (a).space = (a).space * 2;                                        \
        }                                                                     \
        (a).elems = util_realloc_or_die((a).elems,                            \
                                        sizeof((a).elems[0]) * (a).space);    \
    }                                                                         \
    (a).elems[(a).elem_count] = (elem);                                       \
    (a).elem_count++;                                                         \
}

#define ARRAY_DELETE(a) {  \
    free(a.elems);         \
}

#endif /* ARRAY_H */
