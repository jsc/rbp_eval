#include "array.h"

#ifdef ARRAY_MAIN

#include <assert.h>

int main(void) {
    ARRAY_TYPE_DECL(dbl_arr, double);
    ARRAY_TYPE_DECL(int_arr, int);
    int ti[] = {1, 4, 3, 2, 8, 9, 2, 4, 1, 55, -5, 19 };
    double td[] = {3.0, 2.2, 8.1, 9.4, 43.0, -5.3, 3.5, 17.34};
    int i;
    int num_i = sizeof(ti) / sizeof(ti[0]);
    int num_d = sizeof(td) / sizeof(td[0]);

    dbl_arr da;
    int_arr ia;

    ARRAY_INIT(da);
    ARRAY_INIT(ia);

    for (i = 0; i < num_i; i++) {
        ARRAY_ADD(ia, ti[i]);
    }
    assert(ia.elem_count == num_i);

    for (i = 0; i < num_d; i++) {
        ARRAY_ADD(da, td[i]);
    }
    assert(da.elem_count = num_d);;

    for (i = 0; i < num_i; i++) {
        assert(ia.elems[i] == ti[i]);
    }
    for (i = 0; i < num_d; i++) {
        assert(da.elems[i] == td[i]);
    }
    return 0;
}

#endif /* ARRAY_MAIN */
